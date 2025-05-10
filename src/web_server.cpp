#include "web_server.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include "sensors.h"
#include "websocket_client.h"

// HTML for the configuration page
const char* configHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM Configuration</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; }
    .form-group { margin-bottom: 10px; }
    label { display: inline-block; width: 150px; }
    input[type="text"], input[type="number"], input[type="password"] { width: 100px; }
    input[type="submit"] { margin-top: 15px; padding: 8px 16px; }
    .container { max-width: 500px; margin: 0 auto; }
    .tab { overflow: hidden; border: 1px solid #ccc; background-color: #f1f1f1; }
    .tab button { background-color: inherit; float: left; border: none; outline: none; cursor: pointer; padding: 14px 16px; }
    .tab button:hover { background-color: #ddd; }
    .tab button.active { background-color: #ccc; }
    .tabcontent { display: none; padding: 6px 12px; border: 1px solid #ccc; border-top: none; }
    .tabcontent.active { display: block; }
  </style>
</head>
<body>
  <div class="container">
    <h2>ESP32-CAM Robot Configuration</h2>
    
    <div class="tab">
      <button class="tablinks active" onclick="openTab(event, 'WiFiTab')">WiFi Setup</button>
      <button class="tablinks" onclick="openTab(event, 'PinsTab')">Pin Configuration</button>
    </div>
    
    <div id="WiFiTab" class="tabcontent active">
      <h3>WiFi Configuration</h3>
      <form action="/savewifi" method="POST">
        <div class="form-group">
          <label for="ssid">WiFi SSID:</label>
          <input type="text" id="wifi_ssid" name="wifi_ssid" value="%WIFI_SSID%" style="width: 200px;">
        </div>
        <div class="form-group">
          <label for="password">WiFi Password:</label>
          <input type="password" id="wifi_password" name="wifi_password" value="%WIFI_PASS%" style="width: 200px;">
        </div>
        <div class="form-group">
          <label for="ws">WebSocket Server:</label>
          <input type="text" id="ws_server" name="ws_server" value="%WS_SERVER%" style="width: 200px;" placeholder="e.g., ore0.lazos.me">
        </div>
        <div class="form-group">
          <label for="port">WebSocket Port:</label>
          <input type="number" id="ws_port" name="ws_port" value="%WS_PORT%" style="width: 100px;">
        </div>
        <div class="form-group">
          <label for="path">WebSocket Path:</label>
          <input type="text" id="ws_path" name="ws_path" value="%WS_PATH%" style="width: 200px;">
        </div>
        <div class="form-group">
          <label for="ssl">Use SSL:</label>
          <input type="checkbox" id="ws_ssl" name="ws_ssl" %WS_SSL%>
          <label for="ssl">Enable secure WebSocket (wss://)</label>
        </div>
        <input type="submit" value="Save WiFi Settings">
      </form>
    </div>
    
    <div id="PinsTab" class="tabcontent">
      <h3>Pin Configuration</h3>
      <form action="/savepins" method="POST">
        <h4>Motor Control Pins</h4>
        <div class="form-group">
          <label for="m1i1">Motor 1 IN1:</label>
          <input type="number" id="m1i1" name="m1i1" value="%M1I1%">
        </div>
        <div class="form-group">
          <label for="m1i2">Motor 1 IN2:</label>
          <input type="number" id="m1i2" name="m1i2" value="%M1I2%">
        </div>
        <div class="form-group">
          <label for="m2i1">Motor 2 IN1:</label>
          <input type="number" id="m2i1" name="m2i1" value="%M2I1%">
        </div>
        <div class="form-group">
          <label for="m2i2">Motor 2 IN2:</label>
          <input type="number" id="m2i2" name="m2i2" value="%M2I2%">
        </div>
        
        <h4>Ultrasonic Sensor Pins</h4>
        <div class="form-group">
          <label for="sonar_trig">Trigger Pin:</label>
          <input type="number" id="sonar_trig" name="sonar_trig" value="%SONAR_TRIG%">
        </div>
        <div class="form-group">
          <label for="sonar_echo">Echo Pin:</label>
          <input type="number" id="sonar_echo" name="sonar_echo" value="%SONAR_ECHO%">
        </div>
        
        <h4>I2C Pins (DAC)</h4>
        <div class="form-group">
          <label for="sda">SDA Pin:</label>
          <input type="number" id="sda" name="sda" value="%SDA%">
        </div>
        <div class="form-group">
          <label for="scl">SCL Pin:</label>
          <input type="number" id="scl" name="scl" value="%SCL%">
        </div>
        
        <input type="submit" value="Save Pin Configuration">
      </form>
    </div>
    
    <div style="margin-top: 20px;">
      <p>WiFi: <span id="wifi">%WIFI%</span></p>
      <p>IP Address: <span id="ip">%IP%</span></p>
      <p>WebSocket Client: <span id="status">%STATUS%</span></p>
    </div>
  </div>
  
  <script>
    function openTab(evt, tabName) {
      var i, tabcontent, tablinks;
      tabcontent = document.getElementsByClassName("tabcontent");
      for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].className = tabcontent[i].className.replace(" active", "");
      }
      tablinks = document.getElementsByClassName("tablinks");
      for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
      }
      document.getElementById(tabName).className += " active";
      evt.currentTarget.className += " active";
    }
  </script>
</body>
</html>
)rawliteral";

// Template processor function
String processor(const String& var) {
  if (var == "M1I1")        return String(pins.motor1_in1);
  if (var == "M1I2")        return String(pins.motor1_in2);
  if (var == "M2I1")        return String(pins.motor2_in1);
  if (var == "M2I2")        return String(pins.motor2_in2);
  if (var == "SONAR_TRIG")  return String(pins.sonar_trigger);
  if (var == "SONAR_ECHO")  return String(pins.sonar_echo);
  if (var == "SDA")         return String(pins.i2c_sda);
  if (var == "SCL")         return String(pins.i2c_scl);
  if (var == "WS_SERVER")   return wsConfig.server;
  if (var == "WS_PORT")     return String(wsConfig.port);
  if (var == "WS_PATH")     return wsConfig.path;
  if (var == "WS_SSL")      return wsConfig.useSSL ? "true" : "false";
  if (var == "WIFI_SSID")   return wifiConfig.ssid;
  if (var == "WIFI_PASS")   return wifiConfig.password;
  if (var == "STATUS")      return wsConnected ? "Connected to WebSocket server" : "Disconnected";
  if (var == "WIFI")        return WiFi.isConnected() ? "Connected to " + WiFi.SSID() : (apMode ? "AP Mode" : "Disconnected");
  if (var == "IP")          return apMode ? WiFi.softAPIP().toString() : (WiFi.isConnected() ? WiFi.localIP().toString() : "Not connected");
  return String();
}

// Setup web server
void setupWebServer() {
  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req){
    req->send(200, "text/html", configHtml, processor);
  });
  
  // Save WiFi settings
  server.on("/savewifi", HTTP_POST, [](AsyncWebServerRequest* req){    
    saveWiFiConfig(
      req->getParam("wifi_ssid", true)->value(),
      req->getParam("wifi_password", true)->value()
    );
    saveWebSocketConfig(
      req->getParam("ws_server", true)->value(),
      req->getParam("ws_port", true)->value().toInt(),
      req->getParam("ws_path", true)->value(),
      req->hasParam("ws_ssl", true)
    );
    
    req->send(200, "text/plain", "WiFi settings saved. Rebooting...");
    delay(1000);
    ESP.restart();
  });
  
  // Save pin configuration
  server.on("/savepins", HTTP_POST, [](AsyncWebServerRequest* req){
    savePinMap(PinMap{
      (uint8_t)req->getParam("m1i1", true)->value().toInt(),
      (uint8_t)req->getParam("m1i2", true)->value().toInt(),
      (uint8_t)req->getParam("m2i1", true)->value().toInt(),
      (uint8_t)req->getParam("m2i2", true)->value().toInt(),
      (uint8_t)req->getParam("sonar_trig", true)->value().toInt(),
      (uint8_t)req->getParam("sonar_echo", true)->value().toInt(),
      (uint8_t)req->getParam("sda", true)->value().toInt(),
      (uint8_t)req->getParam("scl", true)->value().toInt(),
    });
    req->send(200, "text/plain", "Pin configuration saved. Rebooting...");
    delay(1000);
    ESP.restart();
  });
  
  // API endpoint to get distance from ultrasonic sensor
  server.on("/api/distance", HTTP_GET, [](AsyncWebServerRequest* req){
    float distance = measureDistance();
    String json = "{\"distance\": " + String(distance) + "}";
    req->send(200, "application/json", json);
  });
  
  // API endpoint to get status
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* req){
    String json = "{\"wifi\": \"" + (WiFi.isConnected() ? WiFi.SSID() : "Not connected") + 
                  "\", \"ip\": \"" + (WiFi.isConnected() ? WiFi.localIP().toString() : "Not connected") + 
                  "\", \"wsConnected\": " + String(wsConnected ? "true" : "false") + "," +
                  "\"wsServer\": \"" + wsConfig.useSSL ? "wss://" : "ws://" + wsConfig.server + ":" + String(wsConfig.port) +
                  "\", \"wsPath\": \"" + wsConfig.path + "\"}";
    req->send(200, "application/json", json);
  });
  
  // Start the server
  server.begin();
  Serial.println("Web server started on port " + String(WEB_SERVER_PORT));
}
