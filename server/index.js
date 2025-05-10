const WebSocket = require('ws');
const http = require('http');
const {readFileSync} = require('fs');
const {parse} = require('url');
const {on} = require('events');

const Commands = {
  0x01: 'LM_FORWARD',
  0x02: 'LM_BACKWARD',
  0x03: 'LM_STOP',
  0x04: 'LM_BREAK',
  0x05: 'RM_FORWARD',
  0x06: 'RM_BACKWARD',
  0x07: 'RM_STOP',
  0x08: 'RM_BREAK',
  0x09: 'ALL_STOP',
  0x0A: 'ALL_BREAK',
  0x10: 'MOTOR_CONTROL',
  0x20: 'SET_MOTOR_PINS',
  0x21: 'GET_MOTOR_PINS',
};

const Motor = {LEFT: 0, RIGHT: 1, BOTH: 2};
const MotorDirection = {STOP: 0, FORWARD: 1, BACKWARD: 2, BRAKE: 3};


const server = http.createServer();

const indexHtml = readFileSync(__dirname + '/index.html');

// serve static index.html from file on / path
server.on('request', (req, res) => {
  if (req.url === '/') {
    res.writeHead(200, {'Content-Type': 'text/html'});
    res.end(indexHtml);
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
const clients = [];

wss.on('connection', (ws) => {
  console.log('Client connected');
  clients.push(ws);

  ws.on('message', (data) => {
    const commands = new Uint8Array(data);
    console.log(`Received commands: ${commands}`);
    for (const cmd of commands) {
      const cmdName = Commands[cmd] || 'UNKNOWN';
      // Handle motor control
      switch (cmd) {
        // Left Motor Commands
        case 0x01:
          motorStates.left = 'FORWARD';
          break;
        case 0x02:
          motorStates.left = 'BACKWARD';
          break;
        case 0x03:
          motorStates.left = 'STOPPED';
          break;
        case 0x04:
          motorStates.left = 'BRAKING';
          break;

        // Right Motor Commands
        case 0x05:
          motorStates.right = 'FORWARD';
          break;
        case 0x06:
          motorStates.right = 'BACKWARD';
          break;
        case 0x07:
          motorStates.right = 'STOPPED';
          break;
        case 0x08:
          motorStates.right = 'BRAKING';
          break;

        // Combined Commands
        case 0x09:
          motorStates.left = 'STOPPED';
          motorStates.right = 'STOPPED';
          break;
        case 0x0A:
          motorStates.left = 'BRAKING';
          motorStates.right = 'BRAKING';
          break;
        default:
          console.log(`Unknown command: ${cmdName} (${cmd})`);
          return;
      }
    }
    commandBus.publish(commands);
    console.log(`Motor States: Left=${motorStates.left}, Right=${motorStates.right}`);
  });

  let statusInterval = setInterval(() => {
    const status = JSON.stringify({
      type: 'status',
      online: oreoClient?.readyState === WebSocket.OPEN,
      left: motorStates.left,
      right: motorStates.right
    });
    ws.send(status);
  }, 500);

  ws.on('close', () => {
    clients.splice(clients.indexOf(ws), 1);
    clearInterval(statusInterval);
    console.log('Client disconnected');
  });
});


const wssc = new WebSocket.Server({noServer: true});

wssc.on('connection', (wsc) => {
  console.log('ORE0 connected');
  oreoClient = wsc;

  commandBus.subscribe((commands) => {
    // convert commands (Uint8Array) to comma-separated string
    const state = Array.from(commands).join(',');
    console.log(`Sending commands: ${state}`);
    wsc.send(state);
  });

  // receive binary data
  wsc.on('message', (data) => {
    // binary data is a jpeg frame
    // emit it to all clients as binary data
    clients.forEach(client => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(data, {binary: true});
      }
    });
  });

  wsc.on('close', () => {
    commandBus.listeners.splice(0, commandBus.length);
    oreoClient = null;
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