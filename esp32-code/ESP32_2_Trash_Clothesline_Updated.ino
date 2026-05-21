#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <EEPROM.h>

// EEPROM addresses
#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASS_ADDR 100
#define CONFIGURED_ADDR 200

// Serial communication with ESP32 #4
#define RX_FROM_ESP4 16  // Receive WiFi config from ESP32 #4
#define TX_TO_ESP4 17    // Send logs to ESP32 #4
HardwareSerial SerialToESP4(2); // Use Serial2

// MQTT Broker
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Pin definitions - TRASH SORTING SYSTEM
#define ULTRASONIC_TRIG_ORGANIK 5
#define ULTRASONIC_ECHO_ORGANIK 18
#define ULTRASONIC_TRIG_ANORGANIK 19
#define ULTRASONIC_ECHO_ANORGANIK 21
#define ULTRASONIC_TRIG_METAL 22
#define ULTRASONIC_ECHO_METAL 23

#define COLOR_SENSOR_S0 32
#define COLOR_SENSOR_S1 33
#define COLOR_SENSOR_S2 25
#define COLOR_SENSOR_S3 26
#define COLOR_SENSOR_OUT 27

#define METAL_DETECTOR_PIN 34

#define SERVO_ROTATION_PIN 13
#define SERVO_GATE_PIN 12

// Pin definitions - CLOTHESLINE
#define RAIN_SENSOR_PIN 35
#define CLOTHESLINE_SERVO_PIN 4  // Ganti dari motor ke servo
#define CLOTHESLINE_BUZZER_PIN 2 // Buzzer untuk alert hujan

Servo rotationServo;
Servo gateServo;
Servo clotheslineServo;  // Tambah servo untuk jemuran

// Forward declarations
void rotateToPosition(int angle);
void openClothesline();
void closeClothesline();
void processTrash();
int readTrashLevel(int trigPin, int echoPin);
bool checkRain();
void rainAlert();

// Variables - Trash System
int currentRotation = 0; // 0=Organik, 120=Anorganik, 240=Metal
bool isProcessing = false;
int itemCount[3] = {0, 0, 0}; // Organik, Anorganik, Metal

// Variables - Clothesline
bool clotheslineOpen = true;  // true = 90° (buka), false = 0° (tutup)
bool autoMode = true;
bool isRaining = false;

// Timing
unsigned long lastTrashRead = 0;
unsigned long lastRainRead = 0;
unsigned long lastHeartbeat = 0;
const long trashInterval = 5000;
const long rainInterval = 1000;
const long heartbeatInterval = 10000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("========================================");
  Serial.println("ESP32 #2 - Smart Trash & Clothesline");
  Serial.println("========================================");
  Serial.println("Initializing system...");
  
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("✓ EEPROM initialized");
  
  // Initialize Serial with ESP32 #4
  SerialToESP4.begin(9600, SERIAL_8N1, RX_FROM_ESP4, TX_TO_ESP4); // RX=16, TX=17
  Serial.println("✓ Serial communication with ESP32 #4 initialized");
  
  Serial.println("Setting up trash system pins...");
  // Trash system pins
  pinMode(ULTRASONIC_TRIG_ORGANIK, OUTPUT);
  pinMode(ULTRASONIC_ECHO_ORGANIK, INPUT);
  pinMode(ULTRASONIC_TRIG_ANORGANIK, OUTPUT);
  pinMode(ULTRASONIC_ECHO_ANORGANIK, INPUT);
  pinMode(ULTRASONIC_TRIG_METAL, OUTPUT);
  pinMode(ULTRASONIC_ECHO_METAL, INPUT);
  
  pinMode(COLOR_SENSOR_S0, OUTPUT);
  pinMode(COLOR_SENSOR_S1, OUTPUT);
  pinMode(COLOR_SENSOR_S2, OUTPUT);
  pinMode(COLOR_SENSOR_S3, OUTPUT);
  pinMode(COLOR_SENSOR_OUT, INPUT);
  
  pinMode(METAL_DETECTOR_PIN, INPUT);
  
  // Set color sensor frequency
  digitalWrite(COLOR_SENSOR_S0, HIGH);
  digitalWrite(COLOR_SENSOR_S1, LOW);
  Serial.println("✓ Trash system pins configured");
  
  Serial.println("Initializing servos...");
  rotationServo.attach(SERVO_ROTATION_PIN);
  gateServo.attach(SERVO_GATE_PIN);
  clotheslineServo.attach(CLOTHESLINE_SERVO_PIN);
  
  rotationServo.write(0);
  gateServo.write(0); // Gate closed
  clotheslineServo.write(90); // Jemuran buka (default)
  Serial.println("✓ Servos initialized (Trash: 0°, Gate: 0°, Clothesline: 90°)");
  
  Serial.println("Setting up clothesline system...");
  // Clothesline pins
  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(CLOTHESLINE_BUZZER_PIN, OUTPUT);
  digitalWrite(CLOTHESLINE_BUZZER_PIN, LOW);
  Serial.println("✓ Clothesline system configured");
  Serial.println("  - Rain sensor: GPIO 35 (analog)");
  Serial.println("  - Servo: GPIO 4 (PWM)");
  Serial.println("  - Buzzer: GPIO 2 (digital)");
  
  Serial.println("Connecting to WiFi...");
  // Try to connect to WiFi
  if (connectToWiFi()) {
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    Serial.println("✓ MQTT client configured");
  } else {
    Serial.println("⚠️  Waiting for WiFi config from ESP32 #4...");
  }
  
  Serial.println("========================================");
  Serial.println("🚀 ESP32 #2 Setup Complete!");
  Serial.println("Ready for trash sorting and clothesline control");
  Serial.println("========================================");
}

bool isWiFiConfigured() {
  byte configured = EEPROM.read(CONFIGURED_ADDR);
  return (configured == 1);
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
  Serial.println("ESP32 #2 - WiFi Connection");
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

void sendLogToESP4(String log) {
  SerialToESP4.println("ESP2:" + log);
}

void receiveWiFiConfigFromESP4() {
  if (SerialToESP4.available()) {
    String data = SerialToESP4.readStringUntil('\n');
    data.trim(); // Remove whitespace
    
    // Check for RESET_WIFI command
    if (data == "RESET_WIFI") {
      Serial.println("========================================");
      Serial.println("RESET_WIFI command received!");
      Serial.println("Disconnecting WiFi and clearing config...");
      Serial.println("========================================");
      
      // Send offline status to ESP32 #4 before disconnecting
      sendLogToESP4("WiFi:RESET");
      delay(100);
      
      // Disconnect WiFi
      WiFi.disconnect(true);
      delay(100);
      
      // Clear EEPROM WiFi config
      for (int i = 0; i < 200; i++) {
        EEPROM.write(i, 0);
      }
      EEPROM.write(CONFIGURED_ADDR, 0); // Mark as not configured
      EEPROM.commit();
      
      Serial.println("✓ WiFi config cleared!");
      Serial.println("Waiting for new WiFi config from ESP32 #4...");
      Serial.println("========================================");
      
      // Don't restart, just wait for new config
      return;
    }
    
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

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("📨 MQTT Message Received:");
  Serial.println("  Topic: " + String(topic));
  Serial.println("  Message: " + message);
  Serial.println("  Length: " + String(length) + " bytes");
  
  // Trash rotation control
  if (String(topic) == "iot/trash/rotate") {
    int rotation = message.toInt();
    Serial.println("🗂️  Trash rotation command: " + String(rotation) + "°");
    rotateToPosition(rotation);
  }
  
  if (String(topic) == "iot/trash/reset") {
    Serial.println("🔄 Trash system reset command");
    rotateToPosition(0);
    gateServo.write(0);
  }
  
  // Clothesline control
  if (String(topic) == "iot/clothesline/control") {
    Serial.println("🏠 Clothesline manual control: " + message);
    if (message == "OPEN") {
      openClothesline();
    } else if (message == "CLOSE") {
      closeClothesline();
    }
  }
  
  if (String(topic) == "iot/clothesline/auto") {
    autoMode = (message == "ON");
    Serial.println("⚙️  Auto mode " + String(autoMode ? "ENABLED" : "DISABLED"));
  }
  
  if (String(topic) == "iot/esp32_2/wifi/config") {
    Serial.println("📶 WiFi config received: " + message);
  }
  
  // Ping response
  if (String(topic) == "iot/esp32_2/ping") {
    if (message == "PING") {
      Serial.println("========================================");
      Serial.println("🏓 PING REQUEST RECEIVED");
      Serial.println("From topic: iot/esp32_2/ping");
      Serial.println("Message: " + message);
      Serial.println("Responding with PONG...");
      
      bool published = client.publish("iot/esp32_2/pong", "PONG");
      if (published) {
        Serial.println("✅ PONG sent successfully");
        sendLogToESP4("Ping:OK");
      } else {
        Serial.println("❌ Failed to send PONG");
        sendLogToESP4("Ping:FAIL");
      }
      Serial.println("========================================");
    }
  }
  
  Serial.println("✅ MQTT message processed");
}

void rotateToPosition(int angle) {
  Serial.println("========================================");
  Serial.println("🔄 TRASH ROTATION COMMAND");
  Serial.println("Current position: " + String(currentRotation) + "°");
  Serial.println("Target position: " + String(angle) + "°");
  Serial.println("Rotating servo...");
  
  currentRotation = angle;
  rotationServo.write(angle);
  
  // Wait for servo to reach position
  delay(1000);
  
  // Publish status
  bool published = client.publish("iot/trash/rotation", String(angle).c_str());
  if (published) {
    Serial.println("✅ Rotation completed and status published");
    sendLogToESP4("Rotated:" + String(angle));
  } else {
    Serial.println("❌ Failed to publish rotation status");
  }
  
  Serial.println("Final position: " + String(angle) + "°");
  Serial.println("========================================");
}

String detectTrashType() {
  // Check metal detector first
  int metalValue = digitalRead(METAL_DETECTOR_PIN);
  if (metalValue == HIGH) {
    Serial.println("Metal detected!");
    return "Metal";
  }
  
  // Read color sensor
  int red = readColor('R');
  int green = readColor('G');
  int blue = readColor('B');
  
  Serial.print("RGB: ");
  Serial.print(red);
  Serial.print(", ");
  Serial.print(green);
  Serial.print(", ");
  Serial.println(blue);
  
  // Simple color classification
  // Green/Brown = Organik
  // Other colors = Anorganik
  if (green > red && green > blue) {
    return "Organik";
  } else {
    return "Anorganik";
  }
}

int readColor(char color) {
  if (color == 'R') {
    digitalWrite(COLOR_SENSOR_S2, LOW);
    digitalWrite(COLOR_SENSOR_S3, LOW);
  } else if (color == 'G') {
    digitalWrite(COLOR_SENSOR_S2, HIGH);
    digitalWrite(COLOR_SENSOR_S3, HIGH);
  } else if (color == 'B') {
    digitalWrite(COLOR_SENSOR_S2, LOW);
    digitalWrite(COLOR_SENSOR_S3, HIGH);
  }
  
  int frequency = pulseIn(COLOR_SENSOR_OUT, LOW);
  return frequency;
}

void processTrash() {
  if (isProcessing) return;
  
  isProcessing = true;
  Serial.println("Processing trash...");
  sendLogToESP4("Scanning...");
  client.publish("iot/esp32_2/status", "Scanning trash...");
  
  // Detect trash type
  String trashType = detectTrashType();
  
  // Rotate to correct bin
  int targetRotation = 0;
  int binIndex = 0;
  
  if (trashType == "Organik") {
    targetRotation = 0;
    binIndex = 0;
    sendLogToESP4("Sort:Organik");
    client.publish("iot/esp32_2/status", "Sorting: Organik");
  } else if (trashType == "Anorganik") {
    targetRotation = 120;
    binIndex = 1;
    sendLogToESP4("Sort:Anorganik");
    client.publish("iot/esp32_2/status", "Sorting: Anorganik");
  } else if (trashType == "Metal") {
    targetRotation = 240;
    binIndex = 2;
    sendLogToESP4("Sort:Metal");
    client.publish("iot/esp32_2/status", "Sorting: Metal");
  }
  
  rotateToPosition(targetRotation);
  delay(500);
  
  // Open gate to drop trash
  gateServo.write(90);
  delay(1000);
  gateServo.write(0);
  
  // Update item count
  itemCount[binIndex]++;
  
  // Send detection event
  String detectionData = "{\"type\":\"" + trashType + "\",\"rotation\":" + String(targetRotation) + "}";
  client.publish("iot/trash/detection", detectionData.c_str());
  
  Serial.print("Trash sorted: ");
  Serial.println(trashType);
  
  // Return to idle status
  String status = "Clothesline: ";
  status += clotheslineOpen ? "OPEN" : "CLOSED";
  client.publish("iot/esp32_2/status", status.c_str());
  
  isProcessing = false;
}

int readTrashLevel(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;
  
  int binHeight = 30;
  int fillLevel = map(distance, 0, binHeight, 100, 0);
  fillLevel = constrain(fillLevel, 0, 100);
  
  return fillLevel;
}

void openClothesline() {
  Serial.println("========================================");
  Serial.println("📂 OPENING CLOTHESLINE");
  Serial.println("Servo: 0° → 90° (OPEN position)");
  Serial.println("========================================");
  
  sendLogToESP4("Opening...");
  client.publish("iot/esp32_2/status", "Opening clothesline...");
  
  clotheslineServo.write(90); // Buka jemuran (90 derajat)
  clotheslineOpen = true;
  
  client.publish("iot/clothesline/status", "OPEN");
  client.publish("iot/esp32_2/status", "Clothesline: OPEN");
  sendLogToESP4("Clothesline:OPEN");
  
  Serial.println("✅ Clothesline opened successfully");
}

void closeClothesline() {
  Serial.println("========================================");
  Serial.println("📁 CLOSING CLOTHESLINE");
  Serial.println("Servo: 90° → 0° (CLOSED position)");
  Serial.println("========================================");
  
  sendLogToESP4("Closing...");
  client.publish("iot/esp32_2/status", "Closing clothesline...");
  
  clotheslineServo.write(0); // Tutup jemuran (0 derajat)
  clotheslineOpen = false;
  
  client.publish("iot/clothesline/status", "CLOSED");
  client.publish("iot/esp32_2/status", "Clothesline: CLOSED");
  sendLogToESP4("Clothesline:CLOSED");
  
  Serial.println("✅ Clothesline closed successfully");
}

bool checkRain() {
  int rainValue = analogRead(RAIN_SENSOR_PIN);
  // Rain sensor: LOW value = water detected, HIGH value = dry
  // Threshold: < 2000 = ada air (hujan), >= 2000 = kering
  
  // Debug rain sensor value
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 5000) { // Debug every 5 seconds
    Serial.println("🌧️  Rain Sensor Value: " + String(rainValue) + " (Threshold: 2000)");
    lastDebug = millis();
  }
  
  return rainValue < 2000;
}

void rainAlert() {
  // Buzzer alert pattern: 3 beeps
  Serial.println("🔊 RAIN ALERT - Buzzer activated");
  for (int i = 0; i < 3; i++) {
    Serial.print("Beep " + String(i + 1) + "/3... ");
    digitalWrite(CLOTHESLINE_BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(CLOTHESLINE_BUZZER_PIN, LOW);
    delay(200);
    Serial.println("Done");
  }
  Serial.println("🔊 Rain alert completed");
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("========================================");
    Serial.println("🔌 MQTT CONNECTION ATTEMPT");
    Serial.println("Broker: " + String(mqtt_server) + ":" + String(mqtt_port));
    
    String clientId = "ESP32_2_";
    clientId += String(random(0xffff), HEX);
    Serial.println("Client ID: " + clientId);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("✅ MQTT Connected successfully!");
      Serial.println("Subscribing to topics...");
      
      client.subscribe("iot/trash/rotate");
      client.subscribe("iot/trash/reset");
      client.subscribe("iot/clothesline/control");
      client.subscribe("iot/clothesline/auto");
      client.subscribe("iot/esp32_2/wifi/config");
      client.subscribe("iot/esp32_2/ping");
      
      Serial.println("✅ All topics subscribed");
      
      // Send online status
      client.publish("iot/esp32_2/heartbeat", "ONLINE");
      sendLogToESP4("MQTT:Connected");
      
      Serial.println("🚀 ESP32 #2 is now online and ready!");
      Serial.println("========================================");
    } else {
      Serial.println("❌ MQTT Connection failed");
      Serial.println("Error code: " + String(client.state()));
      Serial.println("Retrying in 5 seconds...");
      Serial.println("========================================");
      sendLogToESP4("MQTT:Failed");
      delay(5000);
    }
  }
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
  
  // Read trash bin levels
  if (currentMillis - lastTrashRead >= trashInterval) {
    lastTrashRead = currentMillis;
    
    int organikLevel = readTrashLevel(ULTRASONIC_TRIG_ORGANIK, ULTRASONIC_ECHO_ORGANIK);
    int anorganikLevel = readTrashLevel(ULTRASONIC_TRIG_ANORGANIK, ULTRASONIC_ECHO_ANORGANIK);
    int metalLevel = readTrashLevel(ULTRASONIC_TRIG_METAL, ULTRASONIC_ECHO_METAL);
    
    client.publish("iot/trash/organik/level", String(organikLevel).c_str());
    client.publish("iot/trash/anorganik/level", String(anorganikLevel).c_str());
    client.publish("iot/trash/metal/level", String(metalLevel).c_str());
    
    // Auto process if trash detected (implement trigger sensor if needed)
    // processTrash();
  }
  
  // Check rain sensor
  if (currentMillis - lastRainRead >= rainInterval) {
    lastRainRead = currentMillis;
    bool currentRainStatus = checkRain();
    
    // Only act if rain status changed
    if (currentRainStatus != isRaining) {
      isRaining = currentRainStatus;
      
      if (isRaining) {
        Serial.println("========================================");
        Serial.println("🌧️  RAIN DETECTED!");
        Serial.println("Rain sensor value < 2000 (wet)");
        Serial.println("Auto mode: " + String(autoMode ? "ON" : "OFF"));
        Serial.println("Clothesline status: " + String(clotheslineOpen ? "OPEN" : "CLOSED"));
        Serial.println("========================================");
        
        client.publish("iot/clothesline/rain", "RAIN");
        sendLogToESP4("Rain detected");
        
        // Sound rain alert
        rainAlert();
        
        if (autoMode && clotheslineOpen) {
          Serial.println("🔄 Auto closing clothesline due to rain...");
          closeClothesline(); // Servo dari 90° ke 0° (tutup)
        } else if (!autoMode) {
          Serial.println("⚠️  Auto mode disabled - Manual control required");
          client.publish("iot/esp32_2/status", "Rain detected - Manual control required");
        } else if (!clotheslineOpen) {
          Serial.println("ℹ️  Clothesline already closed");
          client.publish("iot/esp32_2/status", "Rain detected - Clothesline already closed");
        }
      } else {
        Serial.println("========================================");
        Serial.println("☀️  WEATHER CLEAR");
        Serial.println("Rain sensor value >= 2000 (dry)");
        Serial.println("Rain stopped - Safe to hang clothes");
        Serial.println("========================================");
        
        client.publish("iot/clothesline/rain", "CLEAR");
        sendLogToESP4("Weather clear");
        
        // Auto open when rain stops
        if (autoMode && !clotheslineOpen) {
          Serial.println("🔄 Auto opening clothesline - Weather clear");
          openClothesline(); // Servo dari 0° ke 90° (buka)
        } else if (!autoMode) {
          Serial.println("⚠️  Auto mode disabled - Manual control required");
          client.publish("iot/esp32_2/status", "Weather clear - Manual control required");
        } else if (clotheslineOpen) {
          Serial.println("ℹ️  Clothesline already open");
          client.publish("iot/esp32_2/status", "Weather clear - Clothesline already open");
        }
      }
    }
  }
  
  // Send heartbeat and status
  if (currentMillis - lastHeartbeat >= heartbeatInterval) {
    lastHeartbeat = currentMillis;
    client.publish("iot/esp32_2/heartbeat", "ONLINE");
    
    // Send current status
    String status = "Clothesline: ";
    status += clotheslineOpen ? "OPEN" : "CLOSED";
    client.publish("iot/esp32_2/status", status.c_str());
  }
}
