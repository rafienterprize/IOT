#include <WiFi.h>
#include <PubSubClient.h>
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
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);
DNSServer dnsServer;

// LED Status Indicators (4 LEDs + 1 Buzzer)
#define LED_ESP1_PIN 21     // LED untuk status ESP32 #1 (Blue)
#define LED_ESP2_PIN 22     // LED untuk status ESP32 #2 (Green) 
#define LED_ESP3_PIN 23     // LED untuk status ESP32 #3 (Red)
#define LED_SYSTEM_PIN 19   // LED untuk status system (Yellow)
#define BUZZER_PIN 14       // Buzzer untuk startup dan notifikasi

// Config button
#define CONFIG_BUTTON_PIN 0  // GPIO 0 (BOOT button)

bool isConfigMode = false;
bool buttonPressed = false;
unsigned long buttonPressTime = 0;
const long buttonHoldTime = 3000; // Hold 3 seconds

// LED Status Variables
bool esp1_online = false;
bool esp2_online = false;
bool esp3_online = false;
bool system_ready = false;

// Status from all ESP32s
String esp1_status = "Lamp: OFF";
String esp2_status = "Clothesline: CLOSED";
String esp3_status = "Door: LOCKED";

unsigned long lastHeartbeat = 0;
const long heartbeatInterval = 10000;

// LED Control Functions
void setLED(int pin, bool state) {
  digitalWrite(pin, state ? HIGH : LOW);
}

void setAllLEDs(bool state) {
  setLED(LED_ESP1_PIN, state);
  setLED(LED_ESP2_PIN, state);
  setLED(LED_ESP3_PIN, state);
  setLED(LED_SYSTEM_PIN, state);
}

void playStartupAnimation() {
  Serial.println("========================================");
  Serial.println("🚀 ESP32 #4 - WiFi Controller Starting");
  Serial.println("========================================");
  Serial.println("Playing startup animation...");
  
  // Turn off all LEDs first
  setAllLEDs(false);
  delay(500);
  
  // 3 beeps with system LED
  Serial.println("Phase 1: 3 startup beeps");
  for (int i = 0; i < 3; i++) {
    Serial.println("Beep " + String(i + 1) + "/3");
    setLED(LED_SYSTEM_PIN, true);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    setLED(LED_SYSTEM_PIN, false);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
  
  delay(500);
  
  // LED running animation (bolak-balik 2x)
  Serial.println("Phase 2: LED running animation (2 cycles)");
  for (int cycle = 0; cycle < 2; cycle++) {
    Serial.println("Cycle " + String(cycle + 1) + "/2");
    
    // Forward: ESP1 -> ESP2 -> ESP3 -> SYSTEM
    int leds[] = {LED_ESP1_PIN, LED_ESP2_PIN, LED_ESP3_PIN, LED_SYSTEM_PIN};
    for (int i = 0; i < 4; i++) {
      setAllLEDs(false);
      setLED(leds[i], true);
      delay(300);
    }
    
    // Backward: SYSTEM -> ESP3 -> ESP2 -> ESP1
    for (int i = 3; i >= 0; i--) {
      setAllLEDs(false);
      setLED(leds[i], true);
      delay(300);
    }
  }
  
  delay(500);
  
  // Final: All LEDs ON
  Serial.println("Phase 3: All systems ready - All LEDs ON");
  setAllLEDs(true);
  
  // Victory beep
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("✅ Startup animation completed!");
  Serial.println("🌟 ESP32 #4 WiFi Controller is ready!");
  Serial.println("========================================");
  
  system_ready = true;
}

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
  
  // LED animation: blink system LED while forwarding
  for (int i = 0; i < 5; i++) {
    setLED(LED_SYSTEM_PIN, true);
    delay(100);
    setLED(LED_SYSTEM_PIN, false);
    delay(100);
  }
  
  // Send to ESP32 #1
  SerialToESP1.print(wifiData);
  Serial.println("→ Sent to ESP32 #1");
  setLED(LED_ESP1_PIN, true);
  delay(100);
  
  // Send to ESP32 #2
  SerialToESP2.print(wifiData);
  Serial.println("→ Sent to ESP32 #2");
  setLED(LED_ESP2_PIN, true);
  delay(100);
  
  // Send to ESP32 #3 via GPIO 18
  Serial1.begin(9600, SERIAL_8N1, -1, TX_TO_ESP3);
  Serial1.print(wifiData);
  Serial.println("→ Sent to ESP32 #3");
  setLED(LED_ESP3_PIN, true);
  
  Serial.println("========================================");
  Serial.println("✓ WiFi config forwarded to all slaves!");
  Serial.println("========================================");
  
  // All LEDs on to show completion
  setAllLEDs(true);
}

void startConfigMode() {
  isConfigMode = true;
  Serial.println("========================================");
  Serial.println("Starting Config Mode...");
  Serial.println("========================================");
  
  // LED pattern: blink all LEDs to indicate config mode
  for (int i = 0; i < 3; i++) {
    setAllLEDs(true);
    delay(200);
    setAllLEDs(false);
    delay(200);
  }
  
  // Keep system LED on during config mode
  setLED(LED_SYSTEM_PIN, true);
  
  // Disconnect from current WiFi
  WiFi.disconnect();
  delay(100);
  
  // Start Access Point
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
  
  // LED pattern: ESP1, ESP2, ESP3 blink in sequence during config mode
  setLED(LED_ESP1_PIN, true);
  delay(300);
  setLED(LED_ESP1_PIN, false);
  setLED(LED_ESP2_PIN, true);
  delay(300);
  setLED(LED_ESP2_PIN, false);
  setLED(LED_ESP3_PIN, true);
  delay(300);
  setLED(LED_ESP3_PIN, false);
  setLED(LED_SYSTEM_PIN, true); // Keep system LED on
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
  
  // LED animation: rapid blink to show saving
  for (int i = 0; i < 5; i++) {
    setAllLEDs(true);
    delay(100);
    setAllLEDs(false);
    delay(100);
  }
  
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
  
  // LED success pattern: all LEDs blink together 3 times
  for (int i = 0; i < 3; i++) {
    setAllLEDs(true);
    delay(300);
    setAllLEDs(false);
    delay(300);
  }
  
  Serial.println("🔄 All ESP32 devices restarting...");
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
  
  // LED animation: system LED blinks while connecting
  setAllLEDs(false);
  setLED(LED_SYSTEM_PIN, true);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.print("Attempt " + String(attempt) + "/3: ");
    
    int wait = 0;
    while (WiFi.status() != WL_CONNECTED && wait < 10) {
      // Blink system LED while connecting
      setLED(LED_SYSTEM_PIN, !digitalRead(LED_SYSTEM_PIN));
      delay(500);
      Serial.print(".");
      wait++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✓ WiFi Connected!");
      Serial.println("IP: " + WiFi.localIP().toString());
      Serial.println("========================================");
      
      // Success pattern: all LEDs on
      setAllLEDs(true);
      delay(1000);
      
      return true;
    } else {
      Serial.println(" Failed!");
      // Failure pattern: rapid blink
      for (int i = 0; i < 3; i++) {
        setLED(LED_SYSTEM_PIN, true);
        delay(100);
        setLED(LED_SYSTEM_PIN, false);
        delay(100);
      }
    }
  }
  
  Serial.println("✗ WiFi Failed after 3 attempts");
  Serial.println("========================================");
  
  // Failure pattern: all LEDs blink red (off/on pattern)
  for (int i = 0; i < 5; i++) {
    setAllLEDs(false);
    delay(200);
    setAllLEDs(true);
    delay(200);
  }
  setAllLEDs(false);
  
  return false;
}

// Removed old showIntroAnimation and beep functions - replaced with playStartupAnimation

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  
  // Initialize LED pins
  pinMode(LED_ESP1_PIN, OUTPUT);
  pinMode(LED_ESP2_PIN, OUTPUT);
  pinMode(LED_ESP3_PIN, OUTPUT);
  pinMode(LED_SYSTEM_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CONFIG_BUTTON_PIN, INPUT_PULLUP);
  
  // Turn off all LEDs initially
  setAllLEDs(false);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize Serial TO Slaves (for WiFi config)
  SerialToESP1.begin(9600, SERIAL_8N1, -1, TX_TO_ESP1); // TX only
  SerialToESP2.begin(9600, SERIAL_8N1, -1, TX_TO_ESP2); // TX only
  // ESP32 #3 uses default Serial TX (GPIO 1)
  
  // Initialize Serial FROM Slaves (for logs)
  SerialFromESP1.begin(9600, SERIAL_8N1, RX_FROM_ESP1, -1); // RX only
  SerialFromESP2.begin(9600, SERIAL_8N1, RX_FROM_ESP2, -1); // RX only
  // ESP32 #3 logs on GPIO 2 (will be polled)
  
  // Play startup animation
  playStartupAnimation();
  
  // Try to connect to WiFi
  if (!connectToWiFi()) {
    startConfigMode();
  } else {
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    
    Serial.println("🌐 WiFi connected - System fully operational");
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
  
  // Status from ESP32 #1
  if (String(topic) == "iot/esp32_1/status") {
    if (message == "WiFi Reset" || message.indexOf("WiFi:RESET") >= 0) {
      esp1_online = false;
      esp1_status = "WiFi Reset";
      Serial.println("📴 ESP32 #1: WiFi configuration reset");
    } else {
      esp1_status = message;
      esp1_online = true;
      Serial.println("📊 ESP32 #1 Status: " + message);
    }
  }
  
  if (String(topic) == "iot/esp32_1/heartbeat") {
    if (message == "OFFLINE" || message == "RESET") {
      esp1_online = false;
    } else {
      esp1_online = true;
    }
  }
  
  // Status from ESP32 #2
  if (String(topic) == "iot/esp32_2/status") {
    if (message == "WiFi Reset" || message.indexOf("WiFi:RESET") >= 0) {
      esp2_online = false;
      esp2_status = "WiFi Reset";
      Serial.println("📴 ESP32 #2: WiFi configuration reset");
    } else {
      esp2_status = message;
      esp2_online = true;
      Serial.println("📊 ESP32 #2 Status: " + message);
    }
  }
  
  if (String(topic) == "iot/esp32_2/heartbeat") {
    if (message == "OFFLINE" || message == "RESET") {
      esp2_online = false;
    } else {
      esp2_online = true;
    }
  }
  
  // Status from ESP32 #3
  if (String(topic) == "iot/esp32_3/status") {
    if (message == "WiFi Reset" || message.indexOf("WiFi:RESET") >= 0) {
      esp3_online = false;
      esp3_status = "WiFi Reset";
      Serial.println("📴 ESP32 #3: WiFi configuration reset");
    } else {
      esp3_status = message;
      esp3_online = true;
      Serial.println("📊 ESP32 #3 Status: " + message);
    }
  }
  
  if (String(topic) == "iot/esp32_3/heartbeat") {
    if (message == "OFFLINE" || message == "RESET") {
      esp3_online = false;
    } else {
      esp3_online = true;
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
  
  // Forward to all slaves
  forwardWiFiToSlaves(newSSID, newPassword);
  
  Serial.println("Restarting in 2 seconds...");
  delay(2000);
  ESP.restart();
}

void updateLEDStatus() {
  // Update LED status based on ESP32 online status
  setLED(LED_ESP1_PIN, esp1_online);
  setLED(LED_ESP2_PIN, esp2_online);
  setLED(LED_ESP3_PIN, esp3_online);
  setLED(LED_SYSTEM_PIN, system_ready && (esp1_online || esp2_online || esp3_online));
  
  // Debug LED status
  static unsigned long lastLEDDebug = 0;
  if (millis() - lastLEDDebug > 5000) {
    Serial.println("========================================");
    Serial.println("📊 LED Status Monitor");
    Serial.println("ESP32 #1 (Blue): " + String(esp1_online ? "ON" : "OFF") + " - " + esp1_status);
    Serial.println("ESP32 #2 (Green): " + String(esp2_online ? "ON" : "OFF") + " - " + esp2_status);
    Serial.println("ESP32 #3 (Red): " + String(esp3_online ? "ON" : "OFF") + " - " + esp3_status);
    Serial.println("System (Yellow): " + String(system_ready ? "READY" : "STARTING"));
    Serial.println("========================================");
    lastLEDDebug = millis();
  }
}

void checkConfigButton() {
  // Read button state (LOW = pressed because of pullup)
  if (digitalRead(CONFIG_BUTTON_PIN) == LOW) {
    if (!buttonPressed) {
      // Button just pressed
      buttonPressed = true;
      buttonPressTime = millis();
      
      // LED pattern: show button press with system LED
      setLED(LED_SYSTEM_PIN, true);
      delay(100);
      setLED(LED_SYSTEM_PIN, false);
      
      Serial.println("Config button pressed...");
    } else {
      // Button still held
      unsigned long holdDuration = millis() - buttonPressTime;
      
      if (holdDuration >= buttonHoldTime && !isConfigMode) {
        // Button held for 3 seconds - start config mode
        Serial.println("========================================");
        Serial.println("Config button held for 3 seconds!");
        Serial.println("Resetting all ESP32 WiFi...");
        Serial.println("========================================");
        
        // LED pattern: all LEDs blink rapidly for reset
        Serial.println("🔴 Resetting all ESP32 WiFi configurations...");
        for (int i = 0; i < 10; i++) {
          setAllLEDs(true);
          delay(100);
          setAllLEDs(false);
          delay(100);
        }
        
        // Buzzer alert for reset
        for (int i = 0; i < 2; i++) {
          digitalWrite(BUZZER_PIN, HIGH);
          delay(200);
          digitalWrite(BUZZER_PIN, LOW);
          delay(200);
        }
        
        // Send reset command to all ESP32s via serial
        resetAllESP32WiFi();
        
        // LED feedback: show each ESP32 reset status
        Serial.println("🔄 Waiting for ESP32s to reset...");
        
        // Blink each LED to show reset command sent
        setLED(LED_ESP1_PIN, true);
        delay(500);
        setLED(LED_ESP1_PIN, false);
        
        setLED(LED_ESP2_PIN, true);
        delay(500);
        setLED(LED_ESP2_PIN, false);
        
        setLED(LED_ESP3_PIN, true);
        delay(500);
        setLED(LED_ESP3_PIN, false);
        
        delay(1000);
        
        // Config mode LED pattern
        for (int i = 0; i < 3; i++) {
          setAllLEDs(true);
          delay(300);
          setAllLEDs(false);
          delay(300);
        }
        
        delay(1000);
        
        startConfigMode();
      }
    }
  } else {
    // Button released
    if (buttonPressed) {
      unsigned long holdDuration = millis() - buttonPressTime;
      
      if (holdDuration < buttonHoldTime) {
        // Button released before 3 seconds
        Serial.println("Button released (hold 3s for config)");
        
        // Brief LED flash to acknowledge button press
        setAllLEDs(true);
        delay(200);
        updateLEDStatus(); // Return to normal status
      }
      
      buttonPressed = false;
    }
  }
}

void resetAllESP32WiFi() {
  Serial.println("========================================");
  Serial.println("🔄 RESET ALL ESP32 WiFi Configuration");
  Serial.println("Sending RESET_WIFI command to all ESP32s...");
  Serial.println("========================================");
  
  // Send to ESP32 #1
  SerialToESP1.println("RESET_WIFI");
  SerialToESP1.flush(); // Ensure data is sent
  Serial.println("→ Sent RESET_WIFI to ESP32 #1");
  delay(200);
  
  // Send to ESP32 #2
  SerialToESP2.println("RESET_WIFI");
  SerialToESP2.flush(); // Ensure data is sent
  Serial.println("→ Sent RESET_WIFI to ESP32 #2");
  delay(200);
  
  // Send to ESP32 #3 via GPIO 18 (TX_TO_ESP3)
  // Initialize Serial1 for ESP32 #3 communication
  Serial1.begin(9600, SERIAL_8N1, -1, TX_TO_ESP3);
  delay(100); // Give time for serial to initialize
  Serial1.println("RESET_WIFI");
  Serial1.flush(); // Ensure data is sent
  Serial.println("→ Sent RESET_WIFI to ESP32 #3");
  delay(200);
  
  Serial.println("========================================");
  Serial.println("✅ RESET_WIFI commands sent to all ESP32s");
  Serial.println("All ESP32s should disconnect WiFi and clear config");
  Serial.println("========================================");
}

void readLogsFromSlaves() {
  // Read from ESP32 #1
  if (SerialFromESP1.available()) {
    String log = SerialFromESP1.readStringUntil('\n');
    if (log.startsWith("ESP1:")) {
      String message = log.substring(5); // Remove "ESP1:" prefix
      
      // Check for WiFi reset status
      if (message == "WiFi:RESET") {
        esp1_online = false;
        esp1_status = "WiFi Reset";
        Serial.println("📴 ESP1: WiFi configuration reset");
      } else {
        esp1_status = message;
        esp1_online = true;
        Serial.println("LOG ESP1: " + message);
      }
    }
  }
  
  // Read from ESP32 #2
  if (SerialFromESP2.available()) {
    String log = SerialFromESP2.readStringUntil('\n');
    if (log.startsWith("ESP2:")) {
      String message = log.substring(5); // Remove "ESP2:" prefix
      
      // Check for WiFi reset status
      if (message == "WiFi:RESET") {
        esp2_online = false;
        esp2_status = "WiFi Reset";
        Serial.println("📴 ESP2: WiFi configuration reset");
      } else {
        esp2_status = message;
        esp2_online = true;
        Serial.println("LOG ESP2: " + message);
      }
    }
  }
  
  // Read from ESP32 #3 (GPIO 2)
  // Note: You may need to use SoftwareSerial or another HardwareSerial for this
  // For now, we'll rely on MQTT for ESP32 #3 status
}

// Removed updateLCDWithStatus function - replaced with LED status monitoring

// Removed updateRotatingDisplay function - replaced with LED status monitoring

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
  // Check config button (GPIO 0)
  checkConfigButton();
  
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
  
  // Update LED status indicators
  updateLEDStatus();
}
