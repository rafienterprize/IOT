#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <EEPROM.h>

// EEPROM addresses
#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASS_ADDR 100
#define CONFIGURED_ADDR 200

// Serial communication from ESP32 #3 (Master)
#define RX_FROM_MASTER 16  // Receive WiFi config from ESP32 #3
HardwareSerial SerialFromMaster(2); // Use Serial2

// MQTT Broker
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Serial communication from ESP32 #3 (Master)
#define RX_FROM_MASTER 16  // Receive WiFi config from ESP32 #3
HardwareSerial SerialFromMaster(2); // Use Serial2

// Pin definitions
#define LAMP_PIN 2
#define GAS_SENSOR_PIN 34
#define SERVO_PIN 18

Servo feederServo;

// Variables
bool lampState = false;
String lampTimerOn = "";
String lampTimerOff = "";
unsigned long lastGasRead = 0;
unsigned long lastHeartbeat = 0;
const long gasInterval = 2000;
const long heartbeatInterval = 10000;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  
  // Initialize Serial from Master (ESP32 #3)
  SerialFromMaster.begin(9600, SERIAL_8N1, RX_FROM_MASTER, -1); // RX=16, TX not used
  
  pinMode(LAMP_PIN, OUTPUT);
  digitalWrite(LAMP_PIN, LOW);
  
  feederServo.attach(SERVO_PIN);
  feederServo.write(0);
  
  // Try to connect to WiFi
  if (connectToWiFi()) {
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
  } else {
    Serial.println("Waiting for WiFi config from Master ESP32...");
  }
}

bool connectToWiFi() {
  // Check if configured
  if (!isWiFiConfigured()) {
    Serial.println("WiFi not configured yet. Waiting for config from ESP32 #4...");
    return false;
  }
  
  // Read from EEPROM
  String ssid = "";
  String password = "";
  
  for (int i = 0; i < 32; i++) {
    char c = EEPROM.read(SSID_ADDR + i);
    if (c == '\0') break;
    ssid += c;
  }
  
  for (int i = 0; i < 64; i++) {
    char c = EEPROM.read(PASS_ADDR + i);
    if (c == '\0') break;
    password += c;
  }
  
  Serial.println("========================================");
  Serial.println("ESP32 #1 - WiFi Connection");
  Serial.println("========================================");
  Serial.println("SSID: " + ssid);
  Serial.println("Attempting to connect...");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  // Attempt 3 times only
  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.print("Attempt " + String(attempt) + "/3: ");
    
    int wait = 0;
    while (WiFi.status() != WL_CONNECTED && wait < 10) {
      delay(500);
      Serial.print(".");
      wait++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✓ WiFi Connected!");
      Serial.println("IP Address: " + WiFi.localIP().toString());
      Serial.println("========================================");
      return true;
    } else {
      Serial.println(" Failed!");
    }
  }
  
  Serial.println("✗ WiFi Connection Failed after 3 attempts");
  Serial.println("Waiting for new WiFi config from ESP32 #4...");
  Serial.println("========================================");
  return false;
}

bool isWiFiConfigured() {
  byte configured = EEPROM.read(CONFIGURED_ADDR);
  return (configured == 1);
}

void startConfigMode() {
  isConfigMode = true;
  Serial.println("Starting Config Mode...");
  
  // Start Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // Start DNS server for captive portal
  dnsServer.start(53, "*", WiFi.softAPIP());
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.onNotFound(handleRoot);  // Redirect all to root
  
  server.begin();
  Serial.println("Config web server started");
  Serial.println("Connect to WiFi: ESP32-1-Setup");
  Serial.println("Password: 12345678");
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{font-family:Arial;margin:0;padding:20px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh}";
  html += ".container{max-width:400px;margin:0 auto;background:white;padding:30px;border-radius:15px;box-shadow:0 10px 40px rgba(0,0,0,0.2)}";
  html += "h1{color:#333;text-align:center;margin-bottom:10px;font-size:24px}";
  html += ".subtitle{text-align:center;color:#666;margin-bottom:30px;font-size:14px}";
  html += "input{width:100%;padding:12px;margin:10px 0;border:2px solid #ddd;border-radius:8px;box-sizing:border-box;font-size:14px}";
  html += "input:focus{outline:none;border-color:#667eea}";
  html += "button{width:100%;padding:15px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;border:none;border-radius:8px;cursor:pointer;font-size:16px;font-weight:bold}";
  html += "button:hover{opacity:0.9}";
  html += ".info{background:#e3f2fd;padding:15px;border-radius:8px;margin-bottom:20px;font-size:13px;border-left:4px solid #2196F3}";
  html += ".icon{font-size:48px;text-align:center;margin-bottom:10px}";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<div class='icon'>🏠</div>";
  html += "<h1>ESP32 WiFi Setup</h1>";
  html += "<div class='subtitle'>ESP32 #1 - Lamp, Gas, Feeder</div>";
  html += "<div class='info'>📡 Connect ESP32 to your WiFi network</div>";
  html += "<form action='/save' method='POST'>";
  html += "<input type='text' name='ssid' placeholder='WiFi SSID' required>";
  html += "<input type='password' name='password' placeholder='WiFi Password' required>";
  html += "<button type='submit'>💾 Save & Connect</button>";
  html += "</form></div></body></html>";
  
  server.send(200, "text/html", html);
}

void handleSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  Serial.println("Saving WiFi credentials...");
  Serial.println("SSID: " + ssid);
  
  // Save to EEPROM
  for (int i = 0; i < ssid.length(); i++) {
    EEPROM.write(SSID_ADDR + i, ssid[i]);
  }
  EEPROM.write(SSID_ADDR + ssid.length(), '\0');
  
  for (int i = 0; i < password.length(); i++) {
    EEPROM.write(PASS_ADDR + i, password[i]);
  }
  EEPROM.write(PASS_ADDR + password.length(), '\0');
  
  EEPROM.write(CONFIGURED_ADDR, 1);
  EEPROM.commit();
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='3;url=/'>";
  html += "<style>body{font-family:Arial;text-align:center;padding:50px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;margin:0}";
  html += ".success{background:white;padding:40px;border-radius:15px;max-width:400px;margin:0 auto;box-shadow:0 10px 40px rgba(0,0,0,0.2)}";
  html += ".icon{font-size:64px;margin-bottom:20px}";
  html += "h1{color:#4CAF50;margin:10px 0}";
  html += "p{color:#666;font-size:14px}</style></head><body>";
  html += "<div class='success'><div class='icon'>✓</div><h1>Saved Successfully!</h1>";
  html += "<p>ESP32 is restarting and connecting to WiFi...</p>";
  html += "<p style='font-size:12px;color:#999'>This page will close automatically</p></div></body></html>";
  
  server.send(200, "text/html", html);
  
  delay(2000);
  ESP.restart();
}

void setup_wifi() {
  // This function is now replaced by connectToWiFi()
  // Kept for compatibility
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  
  // Smart Lamp Control
  if (String(topic) == "iot/lamp/control") {
    if (message == "ON") {
      lampState = true;
      digitalWrite(LAMP_PIN, HIGH);
      client.publish("iot/lamp/status", "ON");
      client.publish("iot/esp32_1/status", "Lamp: ON");
      sendLogToESP4("Lamp: ON");
    } else if (message == "OFF") {
      lampState = false;
      digitalWrite(LAMP_PIN, LOW);
      client.publish("iot/lamp/status", "OFF");
      client.publish("iot/esp32_1/status", "Lamp: OFF");
      sendLogToESP4("Lamp: OFF");
    }
  }
  
  // Fish Feeder Control
  if (String(topic) == "iot/feeder/control") {
    if (message == "FEED") {
      feedFish();
    }
  }
  
  // WiFi Configuration from website (individual)
  if (String(topic) == "iot/esp32_1/wifi/config") {
    updateWiFiCredentials(message);
  }
  
  // WiFi Configuration broadcast (all ESP32s)
  if (String(topic) == "iot/all/wifi/config") {
    updateWiFiCredentials(message);
  }
  
  // Ping response
  if (String(topic) == "iot/esp32_1/ping") {
    if (message == "PING") {
      client.publish("iot/esp32_1/pong", "PONG");
    }
  }
}

void updateWiFiCredentials(String message) {
  Serial.println("WiFi config received: " + message);
  
  // Parse JSON: {"ssid":"MyWiFi","password":"12345678"}
  int ssidStart = message.indexOf("\"ssid\":\"") + 8;
  int ssidEnd = message.indexOf("\"", ssidStart);
  String newSSID = message.substring(ssidStart, ssidEnd);
  
  int passStart = message.indexOf("\"password\":\"") + 12;
  int passEnd = message.indexOf("\"", passStart);
  String newPassword = message.substring(passStart, passEnd);
  
  if (newSSID.length() == 0) {
    Serial.println("Invalid WiFi credentials!");
    return;
  }
  
  // Save to EEPROM
  for (int i = 0; i < newSSID.length(); i++) {
    EEPROM.write(SSID_ADDR + i, newSSID[i]);
  }
  EEPROM.write(SSID_ADDR + newSSID.length(), '\0');
  
  for (int i = 0; i < newPassword.length(); i++) {
    EEPROM.write(PASS_ADDR + i, newPassword[i]);
  }
  EEPROM.write(PASS_ADDR + newPassword.length(), '\0');
  
  EEPROM.write(CONFIGURED_ADDR, 1);
  EEPROM.commit();
  
  Serial.println("WiFi credentials updated!");
  Serial.println("New SSID: " + newSSID);
  Serial.println("Restarting in 2 seconds...");
  
  delay(2000);
  ESP.restart();
}

void feedFish() {
  Serial.println("Feeding fish...");
  sendLogToESP4("Feeding...");
  client.publish("iot/esp32_1/status", "Feeding...");
  feederServo.write(90);
  delay(1000);
  feederServo.write(0);
  client.publish("iot/feeder/status", "FED");
  sendLogToESP4("Fed complete");
  // Return to lamp status
  String status = "Lamp: ";
  status += lampState ? "ON" : "OFF";
  client.publish("iot/esp32_1/status", status.c_str());
  sendLogToESP4(status);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_1_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("iot/lamp/control");
      client.subscribe("iot/lamp/timer");
      client.subscribe("iot/feeder/control");
      client.subscribe("iot/feeder/schedule");
      client.subscribe("iot/esp32_1/wifi/config");
      client.subscribe("iot/all/wifi/config");  // Broadcast topic
      client.subscribe("iot/esp32_1/ping");
      
      // Send initial heartbeat
      client.publish("iot/esp32_1/heartbeat", "ONLINE");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void sendLogToESP4(String log) {
  SerialToESP4.println("ESP1:" + log);
}

void receiveWiFiConfigFromESP4() {
  if (SerialToESP4.available()) {
    String data = SerialToESP4.readStringUntil('\n');
    
    // Format: "WIFI:SSID:PASSWORD"
    if (data.startsWith("WIFI:")) {
      data = data.substring(5); // Remove "WIFI:" prefix
      
      int separatorIndex = data.indexOf(':');
      if (separatorIndex > 0) {
        String ssid = data.substring(0, separatorIndex);
        String password = data.substring(separatorIndex + 1);
        
        Serial.println("========================================");
        Serial.println("WiFi Config Received from ESP32 #4!");
        Serial.println("========================================");
        Serial.println("SSID: " + ssid);
        Serial.println("Saving to EEPROM...");
        
        // Save to EEPROM
        for (int i = 0; i < ssid.length(); i++) {
          EEPROM.write(SSID_ADDR + i, ssid[i]);
        }
        EEPROM.write(SSID_ADDR + ssid.length(), '\0');
        
        for (int i = 0; i < password.length(); i++) {
          EEPROM.write(PASS_ADDR + i, password[i]);
        }
        EEPROM.write(PASS_ADDR + password.length(), '\0');
        
        EEPROM.write(CONFIGURED_ADDR, 1);
        EEPROM.commit();
        
        Serial.println("✓ WiFi credentials saved!");
        Serial.println("Restarting ESP32 in 3 seconds...");
        Serial.println("========================================");
        
        delay(3000);
        ESP.restart();
      }
    }
  }
}

void readGasSensor() {
  int gasValue = analogRead(GAS_SENSOR_PIN);
  float gasPPM = map(gasValue, 0, 4095, 0, 1000);
  
  char gasStr[10];
  dtostrf(gasPPM, 4, 2, gasStr);
  client.publish("iot/gas/level", gasStr);
  
  Serial.print("Gas Level: ");
  Serial.print(gasPPM);
  Serial.println(" PPM");
}

void loop() {
  // Check for WiFi config from ESP32 #4
  receiveWiFiConfigFromESP4();
  
  // If not connected to WiFi, keep waiting
  if (WiFi.status() != WL_CONNECTED) {
    delay(100);
    return;
  }
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastGasRead >= gasInterval) {
    lastGasRead = currentMillis;
    readGasSensor();
  }
  
  // Send heartbeat and status
  if (currentMillis - lastHeartbeat >= heartbeatInterval) {
    lastHeartbeat = currentMillis;
    client.publish("iot/esp32_1/heartbeat", "ONLINE");
    
    // Send current status
    String status = "Lamp: ";
    status += lampState ? "ON" : "OFF";
    client.publish("iot/esp32_1/status", status.c_str());
  }
}
