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
const char* mqtt_server = "broker.emqx.io";
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
#define CLOTHESLINE_MOTOR_PIN1 14
#define CLOTHESLINE_MOTOR_PIN2 15

Servo rotationServo;
Servo gateServo;

// Variables - Trash System
int currentRotation = 0; // 0=Organik, 120=Anorganik, 240=Metal
bool isProcessing = false;
int itemCount[3] = {0, 0, 0}; // Organik, Anorganik, Metal

// Variables - Clothesline
bool clotheslineOpen = false;
bool autoMode = true;

// Timing
unsigned long lastTrashRead = 0;
unsigned long lastRainRead = 0;
unsigned long lastHeartbeat = 0;
const long trashInterval = 5000;
const long rainInterval = 1000;
const long heartbeatInterval = 10000;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  
  // Initialize Serial with ESP32 #4
  SerialToESP4.begin(9600, SERIAL_8N1, RX_FROM_ESP4, TX_TO_ESP4); // RX=16, TX=17
  
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
  
  rotationServo.attach(SERVO_ROTATION_PIN);
  gateServo.attach(SERVO_GATE_PIN);
  rotationServo.write(0);
  gateServo.write(0); // Gate closed
  
  // Clothesline pins
  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(CLOTHESLINE_MOTOR_PIN1, OUTPUT);
  pinMode(CLOTHESLINE_MOTOR_PIN2, OUTPUT);
  
  digitalWrite(CLOTHESLINE_MOTOR_PIN1, LOW);
  digitalWrite(CLOTHESLINE_MOTOR_PIN2, LOW);
  
  // Try to connect to WiFi
  if (connectToWiFi()) {
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
  } else {
    Serial.println("Waiting for WiFi config from Master ESP32...");
  }
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
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  
  // Trash rotation control
  if (String(topic) == "iot/trash/rotate") {
    int rotation = message.toInt();
    rotateToPosition(rotation);
  }
  
  if (String(topic) == "iot/trash/reset") {
    rotateToPosition(0);
    gateServo.write(0);
  }
  
  // Clothesline control
  if (String(topic) == "iot/clothesline/control") {
    if (message == "OPEN") {
      openClothesline();
    } else if (message == "CLOSE") {
      closeClothesline();
    }
  }
  
  if (String(topic) == "iot/clothesline/auto") {
    autoMode = (message == "ON");
  }
  
  if (String(topic) == "iot/esp32_2/wifi/config") {
    Serial.println("WiFi config received: " + message);
  }
  
  // Ping response
  if (String(topic) == "iot/esp32_2/ping") {
    if (message == "PING") {
      client.publish("iot/esp32_2/pong", "PONG");
    }
  }
}

void rotateToPosition(int angle) {
  currentRotation = angle;
  rotationServo.write(angle);
  client.publish("iot/trash/rotation", String(angle).c_str());
  Serial.print("Rotated to: ");
  Serial.println(angle);
  delay(1000);
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
  Serial.println("Opening clothesline...");
  sendLogToESP4("Opening...");
  client.publish("iot/esp32_2/status", "Opening clothesline...");
  digitalWrite(CLOTHESLINE_MOTOR_PIN1, HIGH);
  digitalWrite(CLOTHESLINE_MOTOR_PIN2, LOW);
  delay(2000);
  digitalWrite(CLOTHESLINE_MOTOR_PIN1, LOW);
  clotheslineOpen = true;
  client.publish("iot/clothesline/status", "OPEN");
  client.publish("iot/esp32_2/status", "Clothesline: OPEN");
  sendLogToESP4("Clothesline:OPEN");
}

void closeClothesline() {
  Serial.println("Closing clothesline...");
  sendLogToESP4("Closing...");
  client.publish("iot/esp32_2/status", "Closing clothesline...");
  digitalWrite(CLOTHESLINE_MOTOR_PIN1, LOW);
  digitalWrite(CLOTHESLINE_MOTOR_PIN2, HIGH);
  delay(2000);
  digitalWrite(CLOTHESLINE_MOTOR_PIN2, LOW);
  clotheslineOpen = false;
  client.publish("iot/clothesline/status", "CLOSE");
  client.publish("iot/esp32_2/status", "Clothesline: CLOSED");
  sendLogToESP4("Clothesline:CLOSE");
}

bool checkRain() {
  int rainValue = analogRead(RAIN_SENSOR_PIN);
  return rainValue < 2000;
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_2_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("iot/trash/rotate");
      client.subscribe("iot/trash/reset");
      client.subscribe("iot/clothesline/control");
      client.subscribe("iot/clothesline/auto");
      client.subscribe("iot/esp32_2/wifi/config");
      client.subscribe("iot/esp32_2/ping");
      
      client.publish("iot/esp32_2/heartbeat", "ONLINE");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
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
    bool isRaining = checkRain();
    
    if (isRaining) {
      client.publish("iot/clothesline/rain", "RAIN");
      if (autoMode && clotheslineOpen) {
        closeClothesline();
      }
    } else {
      client.publish("iot/clothesline/rain", "CLEAR");
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
