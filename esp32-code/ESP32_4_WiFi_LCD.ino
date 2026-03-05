#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>

// EEPROM addresses
#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASS_ADDR 100
#define CONFIGURED_ADDR 200

// WiFi AP Config
const char* ap_ssid = "ESP32-Setup";
const char* ap_password = "12345678";

// Serial communication to/from Slave ESP32s
#define TX_TO_ESP1 17  // TX WiFi config to ESP32 #1
#define TX_TO_ESP2 5   // TX WiFi config to ESP32 #2
#define TX_TO_ESP3 18  // TX WiFi config to ESP32 #3

#define RX_FROM_ESP1 16  // RX logs from ESP32 #1
#define RX_FROM_ESP2 4   // RX logs from ESP32 #2
#define RX_FROM_ESP3 2   // RX logs from ESP32 #3

HardwareSerial SerialToESP1(1);   // Serial1 for ESP32 #1
HardwareSerial SerialToESP2(2);   // Serial2 for ESP32 #2
// ESP32 #3 uses default Serial TX (GPIO 1)

HardwareSerial SerialFromESP1(1); // Serial1 RX for logs
HardwareSerial SerialFromESP2(2); // Serial2 RX for logs
// Note: We'll use Software Serial or polling for ESP32 #3 logs

// MQTT Broker
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);
DNSServer dnsServer;

// LCD I2C (Address 0x27, 16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Buzzer for intro
#define BUZZER_PIN 14

bool isConfigMode = false;

// Display variables
String currentDisplay = "System Siap Digunakan";
unsigned long lastDisplayUpdate = 0;
const long displayInterval = 3000;
int displayMode = 0; // 0=ESP32_1, 1=ESP32_2, 2=ESP32_3

// Status from all ESP32s
String esp1_status = "Lamp: OFF";
String esp2_status = "Clothesline: CLOSED";
String esp3_status = "Door: LOCKED";
bool esp1_online = false;
bool esp2_online = false;
bool esp3_online = false;

unsigned long lastHeartbeat = 0;
const long heartbeatInterval = 10000;

// WiFi Config Functions
bool isWiFiConfigured() {
  byte configured = EEPROM.read(CONFIGURED_ADDR);
  return (configured == 1);
}

void forwardWiFiToSlaves(String ssid, String password) {
  String wifiData = "WIFI:" + ssid + ":" + password + "\n";
  
  Serial.println("========================================");
  Serial.println("Forwarding WiFi config to all ESP32s...");
  Serial.println("========================================");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Forwarding WiFi");
  lcd.setCursor(0, 1);
  lcd.print("To All ESP32...");
  
  // Send to ESP32 #1
  SerialToESP1.print(wifiData);
  Serial.println("→ Sent to ESP32 #1");
  delay(100);
  
  // Send to ESP32 #2
  SerialToESP2.print(wifiData);
  Serial.println("→ Sent to ESP32 #2");
  delay(100);
  
  // Send to ESP32 #3 via GPIO 18
  Serial1.begin(9600, SERIAL_8N1, -1, TX_TO_ESP3);
  Serial1.print(wifiData);
  Serial.println("→ Sent to ESP32 #3");
  
  Serial.println("========================================");
  Serial.println("✓ WiFi config forwarded to all slaves!");
  Serial.println("========================================");
}

void startConfigMode() {
  isConfigMode = true;
  Serial.println("========================================");
  Serial.println("Starting Config Mode...");
  Serial.println("========================================");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Config Mode");
  lcd.setCursor(0, 1);
  lcd.print("Connect to AP");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  Serial.println("AP SSID: " + String(ap_ssid));
  Serial.println("AP Password: " + String(ap_password));
  Serial.println("AP IP: " + WiFi.softAPIP().toString());
  Serial.println("========================================");
  
  dnsServer.start(53, "*", WiFi.softAPIP());
  
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.onNotFound(handleRoot);
  
  server.begin();
  Serial.println("Web server started!");
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
  html += "<h1>Smart Home WiFi Setup</h1>";
  html += "<div class='subtitle'>ESP32 #4 - WiFi Controller</div>";
  html += "<div class='info'>📡 This will configure WiFi for ALL 4 ESP32 devices</div>";
  html += "<form action='/save' method='POST'>";
  html += "<input type='text' name='ssid' placeholder='WiFi SSID' required>";
  html += "<input type='password' name='password' placeholder='WiFi Password' required>";
  html += "<button type='submit'>💾 Save & Connect All</button>";
  html += "</form></div></body></html>";
  
  server.send(200, "text/html", html);
}

void handleSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  Serial.println("========================================");
  Serial.println("WiFi credentials received!");
  Serial.println("SSID: " + ssid);
  Serial.println("========================================");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Saving WiFi...");
  
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
  
  Serial.println("✓ Saved to EEPROM");
  
  // Forward to slave ESP32s
  forwardWiFiToSlaves(ssid, password);
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Arial;text-align:center;padding:50px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;margin:0}";
  html += ".success{background:white;padding:40px;border-radius:15px;max-width:400px;margin:0 auto;box-shadow:0 10px 40px rgba(0,0,0,0.2)}";
  html += ".icon{font-size:64px;margin-bottom:20px}";
  html += "h1{color:#4CAF50;margin:10px 0}";
  html += "p{color:#666;font-size:14px}</style></head><body>";
  html += "<div class='success'><div class='icon'>✓</div><h1>Saved Successfully!</h1>";
  html += "<p>All ESP32 devices are restarting...</p>";
  html += "<p style='font-size:12px;color:#999'>Please wait 10 seconds</p></div></body></html>";
  
  server.send(200, "text/html", html);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("All ESP32");
  lcd.setCursor(0, 1);
  lcd.print("Restarting...");
  
  delay(3000);
  ESP.restart();
}

bool connectToWiFi() {
  if (!isWiFiConfigured()) {
    Serial.println("WiFi not configured. Starting config mode...");
    return false;
  }
  
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
  Serial.println("ESP32 #4 - WiFi Connection");
  Serial.println("========================================");
  Serial.println("SSID: " + ssid);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
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
      Serial.println("IP: " + WiFi.localIP().toString());
      Serial.println("========================================");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Connected!");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP().toString());
      delay(2000);
      
      return true;
    } else {
      Serial.println(" Failed!");
    }
  }
  
  Serial.println("✗ WiFi Failed after 3 attempts");
  Serial.println("========================================");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Failed!");
  lcd.setCursor(0, 1);
  lcd.print("Starting AP...");
  delay(2000);
  
  return false;
}

void showIntroAnimation() {
  // Buzzer beep sequence
  beep(1);
  delay(200);
  beep(1);
  delay(200);
  beep(3);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Home IoT");
  lcd.setCursor(0, 1);
  lcd.print("ESP32 #4");
  delay(1500);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Controller");
  lcd.setCursor(0, 1);
  lcd.print("& LCD Display");
  delay(1500);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("XI SIJA 1");
  lcd.setCursor(0, 1);
  lcd.print("SMKN 7 SEMARANG");
  delay(2000);
  
  beep(2);
}

void beep(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  
  // Initialize Serial TO Slaves (for WiFi config)
  SerialToESP1.begin(9600, SERIAL_8N1, -1, TX_TO_ESP1); // TX only
  SerialToESP2.begin(9600, SERIAL_8N1, -1, TX_TO_ESP2); // TX only
  // ESP32 #3 uses default Serial TX (GPIO 1)
  
  // Initialize Serial FROM Slaves (for logs)
  SerialFromESP1.begin(9600, SERIAL_8N1, RX_FROM_ESP1, -1); // RX only
  SerialFromESP2.begin(9600, SERIAL_8N1, RX_FROM_ESP2, -1); // RX only
  // ESP32 #3 logs on GPIO 2 (will be polled)
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Show intro animation
  showIntroAnimation();
  
  // Try to connect to WiFi
  if (!connectToWiFi()) {
    startConfigMode();
  } else {
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Siap");
    lcd.setCursor(0, 1);
    lcd.print("Digunakan");
  }
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
  
  // WiFi Configuration from website
  if (String(topic) == "iot/esp32_4/wifi/config" || String(topic) == "iot/all/wifi/config") {
    updateWiFiCredentials(message);
  }
  
  // Ping response
  if (String(topic) == "iot/esp32_4/ping") {
    if (message == "PING") {
      client.publish("iot/esp32_4/pong", "PONG");
    }
  }
  
  // Ping response
  if (String(topic) == "iot/esp32_4/ping") {
    if (message == "PING") {
      client.publish("iot/esp32_4/pong", "PONG");
    }
  }
  
  // Status from ESP32 #1
  if (String(topic) == "iot/esp32_1/status") {
    esp1_status = message;
    updateLCDWithStatus(message);
  }
  
  if (String(topic) == "iot/esp32_1/heartbeat") {
    esp1_online = true;
  }
  
  // Status from ESP32 #2
  if (String(topic) == "iot/esp32_2/status") {
    esp2_status = message;
    updateLCDWithStatus(message);
  }
  
  if (String(topic) == "iot/esp32_2/heartbeat") {
    esp2_online = true;
  }
  
  // Status from ESP32 #3
  if (String(topic) == "iot/esp32_3/status") {
    esp3_status = message;
    updateLCDWithStatus(message);
  }
  
  if (String(topic) == "iot/esp32_3/heartbeat") {
    esp3_online = true;
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
  
  // Forward to all slaves
  forwardWiFiToSlaves(newSSID, newPassword);
  
  Serial.println("Restarting in 2 seconds...");
  delay(2000);
  ESP.restart();
}

void updateLCDWithStatus(String status) {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  // Show which ESP32 and status
  if (status.startsWith("Lamp:") || status.startsWith("Feeding")) {
    lcd.print("ESP32 1:");
    lcd.setCursor(0, 1);
    lcd.print(status);
  } else if (status.startsWith("Clothesline:") || status.startsWith("Sorting:") || status.startsWith("Scanning")) {
    lcd.print("ESP32 2:");
    lcd.setCursor(0, 1);
    lcd.print(status);
  } else if (status.startsWith("Door:") || status.startsWith("Access")) {
    lcd.print("ESP32 3:");
    lcd.setCursor(0, 1);
    lcd.print(status);
  } else {
    lcd.print(status.substring(0, 16));
    if (status.length() > 16) {
      lcd.setCursor(0, 1);
      lcd.print(status.substring(16, 32));
    }
  }
  
  currentDisplay = status;
  lastDisplayUpdate = millis();
}

void readLogsFromSlaves() {
  // Read from ESP32 #1
  if (SerialFromESP1.available()) {
    String log = SerialFromESP1.readStringUntil('\n');
    if (log.startsWith("ESP1:")) {
      String message = log.substring(5); // Remove "ESP1:" prefix
      esp1_status = message;
      updateLCDWithStatus("ESP32 1", message);
      Serial.println("LOG ESP1: " + message);
    }
  }
  
  // Read from ESP32 #2
  if (SerialFromESP2.available()) {
    String log = SerialFromESP2.readStringUntil('\n');
    if (log.startsWith("ESP2:")) {
      String message = log.substring(5); // Remove "ESP2:" prefix
      esp2_status = message;
      updateLCDWithStatus("ESP32 2", message);
      Serial.println("LOG ESP2: " + message);
    }
  }
  
  // Read from ESP32 #3 (GPIO 2)
  // Note: You may need to use SoftwareSerial or another HardwareSerial for this
  // For now, we'll rely on MQTT for ESP32 #3 status
}

void updateLCDWithStatus(String device, String status) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(device + ":");
  lcd.setCursor(0, 1);
  lcd.print(status.substring(0, 16)); // Max 16 chars
  
  lastDisplayUpdate = millis();
}

void updateRotatingDisplay() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = currentMillis;
    displayMode = (displayMode + 1) % 3;
    
    lcd.clear();
    
    switch (displayMode) {
      case 0: // ESP32 #1 Status
        lcd.setCursor(0, 0);
        lcd.print("ESP32 1:");
        lcd.print(esp1_online ? "OK" : "X");
        lcd.setCursor(0, 1);
        lcd.print(esp1_status);
        break;
        
      case 1: // ESP32 #2 Status
        lcd.setCursor(0, 0);
        lcd.print("ESP32 2:");
        lcd.print(esp2_online ? "OK" : "X");
        lcd.setCursor(0, 1);
        lcd.print(esp2_status);
        break;
        
      case 2: // ESP32 #3 Status
        lcd.setCursor(0, 0);
        lcd.print("ESP32 3:");
        lcd.print(esp3_online ? "OK" : "X");
        lcd.setCursor(0, 1);
        lcd.print(esp3_status);
        break;
    }
    
    // Reset online status
    esp1_online = false;
    esp2_online = false;
    esp3_online = false;
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_4_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      
      // Subscribe to WiFi config
      client.subscribe("iot/esp32_4/wifi/config");
      client.subscribe("iot/all/wifi/config");
      client.subscribe("iot/esp32_4/ping");
      
      // Subscribe to all ESP32 status (backup via MQTT)
      client.subscribe("iot/esp32_1/status");
      client.subscribe("iot/esp32_1/heartbeat");
      client.subscribe("iot/esp32_2/status");
      client.subscribe("iot/esp32_2/heartbeat");
      client.subscribe("iot/esp32_3/status");
      client.subscribe("iot/esp32_3/heartbeat");
      
      client.publish("iot/esp32_4/heartbeat", "ONLINE");
      client.publish("iot/esp32_4/status", "System Ready");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  // If in config mode, handle web server
  if (isConfigMode) {
    dnsServer.processNextRequest();
    server.handleClient();
    return;
  }
  
  // Read logs from all ESP32s via serial
  readLogsFromSlaves();
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long currentMillis = millis();
  
  // Send heartbeat
  if (currentMillis - lastHeartbeat >= heartbeatInterval) {
    lastHeartbeat = currentMillis;
    client.publish("iot/esp32_4/heartbeat", "ONLINE");
  }
  
  // Update rotating display (only if no recent activity)
  if (currentMillis - lastDisplayUpdate >= displayInterval) {
    updateRotatingDisplay();
  }
}
