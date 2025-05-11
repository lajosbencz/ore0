const WebSocket = require('ws');
const http = require('http');
const {readFileSync} = require('fs');
const {parse} = require('url');
const {on} = require('events');

// Command types from commands.h
const Commands = {
  // Motor control commands
  CMD_MOTOR_CONTROL: 0x10,   // Command to control motors
  
  // Motor pin configuration commands
  CMD_SET_MOTOR_PINS: 0x20,  // Command to set motor pins
  CMD_GET_MOTOR_PINS: 0x21,  // Command to get current motor pin configuration
  
  // GPIO control commands
  CMD_SET_GPIO: 0x30,        // Command to set GPIO pin state
  CMD_GET_GPIO: 0x31         // Command to get GPIO pin state
};

// Motor selection values
const Motor = {
  LEFT: 0,  // MOTOR_CMD_LEFT
  RIGHT: 1, // MOTOR_CMD_RIGHT
  BOTH: 2   // MOTOR_CMD_BOTH
};

// Motor direction values
const MotorDirection = {
  STOP: 0,     // MOTOR_CMD_STOP
  FORWARD: 1,  // MOTOR_CMD_FORWARD
  BACKWARD: 2, // MOTOR_CMD_BACKWARD
  BRAKE: 3     // MOTOR_CMD_BRAKE
};


const server = http.createServer();

const indexHtml = readFileSync(__dirname + '/index.html');
const configHtml = readFileSync(__dirname + '/config.html');

// serve static HTML files
server.on('request', (req, res) => {
  if (req.url === '/') {
    res.writeHead(200, {'Content-Type': 'text/html'});
    res.end(indexHtml);
  } else if (req.url === '/config') {
    res.writeHead(200, {'Content-Type': 'text/html'});
    res.end(configHtml);
  }
});

const wss = new WebSocket.Server({noServer: true});

// Motor state tracking
const motorStates = {
  left: 'STOPPED',  // STOPPED, FORWARD, BACKWARD, BRAKING
  right: 'STOPPED'
};

// event bus for motor state changes between wss and wssc
const commandBus = {
  listeners: [],
  subscribe(listener) {
    this.listeners.push(listener);
  },
  publish(commands) {
    this.listeners.forEach(listener => listener(commands));
  },
  remove(listener) {
    this.listeners = this.listeners.filter(l => l !== listener);
  }
};

let oreoClient = null;
let lastOreoMessageTime = 0; // Track the last time a message was received from ORE0
const clients = [];

// Function to check if there are any clients connected
function hasConnectedClients() {
  return clients.length > 0;
}

// Function to send camera control commands to ESP32
function sendCameraCommand(command) {
  if (oreoClient && oreoClient.readyState === WebSocket.OPEN) {
    console.log(`Sending ${command} command to ESP32`);
    oreoClient.send(command);
  }
}

wss.on('connection', (ws) => {
  console.log('Client connected');
  
  // If this is the first client and ESP32 is connected, start the camera stream
  const isFirstClient = clients.length === 0;
  clients.push(ws);
  
  if (isFirstClient && oreoClient && oreoClient.readyState === WebSocket.OPEN) {
    console.log('First client connected, starting camera stream');
    sendCameraCommand('START_STREAM');
  }

  ws.on('message', (data) => {
    const commands = new Uint8Array(data);
    console.log(`Received commands: ${Array.from(commands).map(c => '0x' + c.toString(16)).join(',')}`);
    
    // Handle structured commands
    if (commands.length >= 3 && commands[0] === Commands.CMD_MOTOR_CONTROL) {
      // Parse motor control command
      const motorCmd = {
        command: commands[0],  // CMD_MOTOR_CONTROL
        motor: commands[1],    // MOTOR_CMD_LEFT, MOTOR_CMD_RIGHT, or MOTOR_CMD_BOTH
        direction: commands[2] // MOTOR_CMD_STOP, MOTOR_CMD_FORWARD, MOTOR_CMD_BACKWARD, or MOTOR_CMD_BRAKE
      };
      
      // Update motor states based on the command
      const updateMotorState = (motorType, direction) => {
        const state = direction === MotorDirection.STOP ? 'STOPPED' :
                     direction === MotorDirection.FORWARD ? 'FORWARD' :
                     direction === MotorDirection.BACKWARD ? 'BACKWARD' :
                     direction === MotorDirection.BRAKE ? 'BRAKING' : 'UNKNOWN';
        
        if (motorType === Motor.LEFT || motorType === Motor.BOTH) {
          motorStates.left = state;
        }
        if (motorType === Motor.RIGHT || motorType === Motor.BOTH) {
          motorStates.right = state;
        }
      };
      
      updateMotorState(motorCmd.motor, motorCmd.direction);
      
      // Forward the structured command to ESP32
      if (oreoClient && oreoClient.readyState === WebSocket.OPEN) {
        commandBus.publish(commands);
      }
    }
    else if (commands.length >= 1) {
      // Other command types can be handled here
      console.log(`Received command type: 0x${commands[0].toString(16)}`);
      
      // Forward all other commands to ESP32
      if (oreoClient && oreoClient.readyState === WebSocket.OPEN) {
        commandBus.publish(commands);
      }
    }
    
    console.log(`Motor States: Left=${motorStates.left}, Right=${motorStates.right}`);
  });

  let statusInterval = setInterval(() => {
    // Consider ORE0 offline if no messages received in the last 5 seconds
    const currentTime = Date.now();
    const isOnline = oreoClient?.readyState === WebSocket.OPEN && 
                     (currentTime - lastOreoMessageTime < 5000);
    
    const status = JSON.stringify({
      type: 'status',
      online: isOnline,
      left: motorStates.left,
      right: motorStates.right
    });
    ws.send(status);
  }, 500);

  ws.on('close', () => {
    clients.splice(clients.indexOf(ws), 1);
    clearInterval(statusInterval);
    console.log('Client disconnected');
    
    // Check if this was the last client
    if (!hasConnectedClients()) {
      console.log('No clients connected, stopping camera stream');
      // Send STOP_STREAM command to ESP32 to stop the camera
      sendCameraCommand('STOP_STREAM');
    }
  });
});


const wssc = new WebSocket.Server({noServer: true});

wssc.on('connection', (wsc) => {
  console.log('ORE0 connected');
  oreoClient = wsc;
  lastOreoMessageTime = Date.now(); // Initialize the last message time

  // Only start streaming if there are clients connected
  if (hasConnectedClients()) {
    console.log('Clients connected, sending START_STREAM command to ESP32');
    wsc.send('START_STREAM');
  } else {
    console.log('No clients connected, not starting camera stream');
  }

  commandBus.subscribe((commands) => {
    // Forward commands to the ESP32
    console.log(`Sending commands to ESP32: ${Array.from(commands).map(c => '0x' + c.toString(16)).join(',')}`);
    wsc.send(commands);
  });

  // Track current motor pin configuration
  let motorPins = {
    m1p1: 12, // Default values
    m1p2: 13,
    m2p1: 15,
    m2p2: 14
  };

  // Track distance measurement from ultrasonic sensor
  let distance = -1;

  // receive binary data or text messages
  wsc.on('message', (data) => {
    // Update the last message time whenever any message is received
    lastOreoMessageTime = Date.now();
    
    // Check if data is a text message (JSON)
    if (typeof data === 'string') {
      try {
        const jsonData = JSON.parse(data);
        console.log(`Received JSON data from ESP32:`, jsonData);
        
        // Handle status message with distance data
        if (jsonData.type === 'status' && jsonData.hasOwnProperty('distance')) {
          distance = jsonData.distance;
          console.log(`Updated distance measurement: ${distance} cm`);
          
          // Forward the status message to all clients
          clients.forEach(client => {
            if (client.readyState === WebSocket.OPEN) {
              // Add motor states to the status message
              const statusMsg = JSON.stringify({
                ...jsonData,
                left: motorStates.left,
                right: motorStates.right
              });
              client.send(statusMsg);
            }
          });
        }
      } catch (e) {
        console.error('Error parsing JSON message:', e);
      }
    }
    // Check if data is a command or a camera frame
    else if (data instanceof Buffer && data.length > 1000) {
      // Likely a camera frame (JPEG images are typically larger than commands)
      console.log(`Received camera frame: ${data.length} bytes`);
      
      // Forward camera frame to all clients
      clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
          client.send(data, {binary: true});
        }
      });
    } else if (data instanceof Buffer && data.length >= 5 && data[0] === Commands.CMD_SET_MOTOR_PINS) {
      // Response to SET_MOTOR_PINS command
      console.log(`Received motor pin configuration from ESP32`);
      
      // Update stored pin configuration
      motorPins = {
        m1p1: data[1],
        m1p2: data[2],
        m2p1: data[3],
        m2p2: data[4]
      };
      
      // Notify all clients about the updated pin configuration
      const pinMsg = JSON.stringify({
        type: 'motor_pins',
        ...motorPins
      });
      
      clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
          client.send(pinMsg);
        }
      });
    } else if (data instanceof Buffer && data.length >= 3 && data[0] === Commands.CMD_GET_GPIO) {
      // Response to GET_GPIO or SET_GPIO command
      console.log(`Received GPIO state from ESP32: pin=${data[1]}, state=${data[2]}`);
      
      // Notify all clients about the GPIO state
      const gpioMsg = JSON.stringify({
        type: 'gpio_state',
        pin: data[1],
        state: data[2]
      });
      
      clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
          client.send(gpioMsg);
        }
      });
    } else {
      // Other command response
      console.log(`Received data from ESP32: ${data.length} bytes`);
      
      // Handle other command responses if needed
    }
  });

  wsc.on('close', () => {
    commandBus.listeners.splice(0, commandBus.length);
    oreoClient = null;
    lastOreoMessageTime = 0; // Reset the last message time
    console.log('ORE0 disconnected');
  });
});


server.on('upgrade', (request, socket, head) => {
  const pathname = parse(request.url).pathname;
  if (pathname === '/ws') {
    wss.handleUpgrade(request, socket, head, (ws) => {
      wss.emit('connection', ws);
    });
  } else if (pathname === '/wsc') {
    wssc.handleUpgrade(request, socket, head, (ws) => {
      wssc.emit('connection', ws);
    });
  } else {
    socket.destroy();
  }
});

server.listen(8080, '0.0.0.0', () => {
  console.log('Server running on ws://localhost:8080');
});
