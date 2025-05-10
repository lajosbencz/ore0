#include "wifi_manager.h"
#include "motor.h"

// AP mode credentials
const char* AP_SSID = "ORE0-CONFIG";
const char* AP_PASSWORD = "ore0";

// Initialize global objects
DNSServer dnsServer;
WebServer server(80);
Preferences preferences;

// Default WebSocket server settings
const char* DEFAULT_WS_HOST = "ore0.lazos.me";
const uint16_t DEFAULT_WS_PORT = 443;
const char* DEFAULT_WS_PATH = "/wsc";
const bool DEFAULT_WS_TLS = true;

bool connectToWiFi() {
  // Get WiFi credentials from preferences
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  
  if (ssid.length() == 0) {
    Serial.println("No WiFi credentials stored");
    return false;
  }
  
  Serial.printf("Connecting to WiFi: %s\n", ssid.c_str());
  
  WiFi.begin(ssid.c_str(), password.c_str());
  WiFi.setSleep(false);
  
  // Wait for connection with timeout
  int timeout = 0;
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  Serial.println("");
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected. IP: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("WiFi connection failed");
    return false;
  }
}

void setupAP() {
  Serial.println("Setting up AP mode");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD, 3, 0, 1);
  
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // Start DNS server for captive portal
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  
  // Set up web server
  server.on("/", HTTP_GET, handleWiFiConfigRoot);
  server.on("/save", HTTP_POST, handleWiFiConfigSave);
  server.onNotFound([]() {
    server.sendHeader("Location", "http://" + WiFi.softAPIP().toString(), true);
    server.send(302, "text/plain", "");
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

void handleWiFiConfigRoot() {
  // Get current WebSocket server settings if they exist
  String wsHost = preferences.getString("ws_host", DEFAULT_WS_HOST);
  uint16_t wsPort = preferences.getUInt("ws_port", DEFAULT_WS_PORT);
  String wsPath = preferences.getString("ws_path", DEFAULT_WS_PATH);
  bool wsTls = preferences.getBool("ws_tls", DEFAULT_WS_TLS);
  
  String html = "<html><head><title>ESP32-CAM WiFi Setup</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Arial;margin:0;padding:20px;text-align:center;}";
  html += "input{width:100%;padding:10px;margin:10px 0;box-sizing:border-box;}";
  html += "button{background-color:#4CAF50;color:white;padding:10px 20px;border:none;cursor:pointer;}";
  html += "fieldset{margin:20px 0;padding:15px;border:1px solid #ddd;border-radius:5px;}";
  html += "legend{font-weight:bold;}</style>";
  html += "</head><body><h1>ESP32-CAM WiFi Setup</h1>";
  html += "<form method='post' action='/save'>";
  
  // WiFi settings section
  html += "<fieldset><legend>WiFi Settings</legend>";
  html += "<label for='ssid'>WiFi SSID:</label><input type='text' id='ssid' name='ssid'><br>";
  html += "<label for='password'>WiFi Password:</label><input type='password' id='password' name='password'><br>";
  html += "</fieldset>";
  
  // WebSocket server settings section
  html += "<fieldset><legend>WebSocket Server Settings</legend>";
  html += "<label for='ws_host'>Server Host:</label><input type='text' id='ws_host' name='ws_host' value='" + wsHost + "'><br>";
  html += "<label for='ws_port'>Server Port:</label><input type='number' id='ws_port' name='ws_port' value='" + String(wsPort) + "'><br>";
  html += "<label for='ws_path'>Server Path:</label><input type='text' id='ws_path' name='ws_path' value='" + wsPath + "'><br>";
  html += "<label for='ws_tls'>Use TLS/SSL:</label>";
  html += "<select id='ws_tls' name='ws_tls'>";
  html += wsTls ? "<option value='1' selected>Yes</option><option value='0'>No</option>" : 
                  "<option value='1'>Yes</option><option value='0' selected>No</option>";
  html += "</select><br>";
  html += "</fieldset>";
  
  
  html += "<button type='submit'>Save and Connect</button>";
  html += "</form></body></html>";
  
  server.send(200, "text/html", html);
}

void handleWiFiConfigSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  // Get WebSocket server settings from form
  String wsHost = server.arg("ws_host");
  String wsPortStr = server.arg("ws_port");
  String wsPath = server.arg("ws_path");
  String wsTlsStr = server.arg("ws_tls");
  
  if (ssid.length() > 0) {
    // Save WiFi credentials to preferences
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    
    // Save WebSocket server settings to preferences
    if (wsHost.length() > 0) {
      preferences.putString("ws_host", wsHost);
    } else {
      preferences.putString("ws_host", DEFAULT_WS_HOST);
    }
    
    uint16_t wsPort = DEFAULT_WS_PORT;
    if (wsPortStr.length() > 0) {
      wsPort = wsPortStr.toInt();
    }
    preferences.putUInt("ws_port", wsPort);
    
    if (wsPath.length() > 0) {
      preferences.putString("ws_path", wsPath);
    } else {
      preferences.putString("ws_path", DEFAULT_WS_PATH);
    }
    
    bool wsTls = DEFAULT_WS_TLS;
    if (wsTlsStr.length() > 0) {
      wsTls = (wsTlsStr == "1");
    }
    preferences.putBool("ws_tls", wsTls);
    
    String html = "<html><head><title>Settings Saved</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:0;padding:20px;text-align:center;}</style>";
    html += "<meta http-equiv='refresh' content='10;url=/'>";
    html += "</head><body><h1>Settings Saved</h1>";
    html += "<p>The device will now attempt to connect to your WiFi network.</p>";
    html += "<p>If connection is successful, this access point will be disabled.</p>";
    html += "<p>Restarting in 10 seconds...</p></body></html>";
    
    server.send(200, "text/html", html);
    
    // Wait a bit before restarting
    preferences.end();
    delay(2000);
    ESP.restart();
  } else {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  }
}

void handleWiFiClient() {
  // In AP mode, handle DNS and web server
  if (WiFi.status() != WL_CONNECTED) {
    dnsServer.processNextRequest();
    server.handleClient();
  }
}

// WebSocket server configuration getters
String getWsHost() {
  return preferences.getString("ws_host", DEFAULT_WS_HOST);
}

uint16_t getWsPort() {
  return preferences.getUInt("ws_port", DEFAULT_WS_PORT);
}

String getWsPath() {
  return preferences.getString("ws_path", DEFAULT_WS_PATH);
}

bool getWsTls() {
  return preferences.getBool("ws_tls", DEFAULT_WS_TLS);
}
