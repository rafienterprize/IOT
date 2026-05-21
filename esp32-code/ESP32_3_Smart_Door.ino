#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include <ESP32Servo.h>

// EEPROM addresses
#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASS_ADDR 100
#define CONFIGURED_ADDR 200

// Serial communication with ESP32 #4
#define RX_FROM_ESP4 16
#define TX_TO_ESP4 17
HardwareSerial SerialToESP4(2);

// MQTT Broker
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// RFID RC522 Pins
#define RST_PIN 22
#define SS_PIN 21

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Door Lock Pins (IR sensor only)
#define DOOR_SERVO_PIN 13
#define DOOR_IR_SENSOR_PIN 14
#define BUZZER_PIN 15

Servo doorServo;

// Smart Gate Pins
#define GATE_SERVO_LEFT_PIN 25
#define GATE_SERVO_RIGHT_PIN 26
#define GATE_IR_SENSOR_PIN 32  // Infrared sensor untuk deteksi mobil (pindah dari 27)

Servo gateServoLeft;
Servo gateServoRight;

// Variables
bool isDoorLocked = true;
bool isGateOpen = false;
String registeredCards[10];
int cardCount = 0;
bool isScanMode = false;

unsigned long lastHeartbeat = 0;
const long heartbeatInterval = 10000;
unsigned long lastGateCheck = 0;
const long gateCheckInterval = 500;
unsigned long lastDoorCheck = 0;
const long doorCheckInterval = 500;

bool isWiFiConfigured() {
  byte configured = EEPROM.read(CONFIGURED_ADDR);
  return (configured == 1);
}

bool connectToWiFi() {
  if (!isWiFiConfigured()) {
    Serial.println("WiFi not configured yet. Waiting for config from ESP32 #4...");
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
  Serial.println("ESP32 #3 - WiFi Connection");
  Serial.println("========================================");
  Serial.println("SSID: " + ssid);
  Serial.println("Attempting to connect...");
  
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
  SerialToESP4.println("ESP3:" + log);
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
      data = data.substring(5);
      
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

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("========================================");
  Serial.println("ESP32 #3 - Smart Door (IR) & Gate (RFID+IR)");
  Serial.println("========================================");
  
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("✓ EEPROM initialized");
  
  SerialToESP4.begin(9600, SERIAL_8N1, RX_FROM_ESP4, TX_TO_ESP4);
  Serial.println("✓ Serial to ESP32 #4 initialized");
  
  Serial.println("Initializing SPI and RFID...");
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("✓ RFID initialized");
  
  Serial.println("Initializing door IR sensor and servo...");
  pinMode(DOOR_IR_SENSOR_PIN, INPUT);
  doorServo.attach(DOOR_SERVO_PIN);
  doorServo.write(0); // Locked position
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.println("✓ Door IR sensor and servo initialized");
  
  Serial.println("Initializing gate IR sensor and servos...");
  pinMode(GATE_IR_SENSOR_PIN, INPUT);
  gateServoLeft.attach(GATE_SERVO_LEFT_PIN);
  gateServoRight.attach(GATE_SERVO_RIGHT_PIN);
  Serial.println("✓ Gate IR sensor and servos initialized");
  
  Serial.println("Closing gate...");
  closeGate();
  Serial.println("✓ Gate closed");
  
  Serial.println("Connecting to WiFi...");
  if (connectToWiFi()) {
    Serial.println("✓ WiFi connected in setup");
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    Serial.println("✓ MQTT client configured in setup");
  } else {
    Serial.println("⚠ WiFi not connected, waiting for config from ESP32 #4...");
  }
  
  Serial.println("========================================");
  Serial.println("Setup complete! Ready for IR detection...");
  Serial.println("========================================");
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
  
  // Manual door control
  if (String(topic) == "iot/door/control") {
    if (message == "LOCK") {
      lockDoor();
    } else if (message == "UNLOCK") {
      unlockDoor();
    }
  }
  
  // Emergency unlock
  if (String(topic) == "iot/door/emergency") {
    unlockDoor();
    beep(3);
  }
  
  // Ping response
  if (String(topic) == "iot/esp32_3/ping") {
    if (message == "PING") {
      client.publish("iot/esp32_3/pong", "PONG");
    }
  }
  
  // Smart Gate Control
  if (String(topic) == "iot/gate/control") {
    if (message == "OPEN") {
      openGate();
    } else if (message == "CLOSE") {
      closeGate();
    }
  }
  
  // Gate RFID registration
  if (String(topic) == "iot/gate/rfid/register") {
    int uidStart = message.indexOf("\"uid\":\"") + 7;
    int uidEnd = message.indexOf("\"", uidStart);
    String uid = message.substring(uidStart, uidEnd);
    
    int nameStart = message.indexOf("\"name\":\"") + 8;
    int nameEnd = message.indexOf("\"", nameStart);
    String name = message.substring(nameStart, nameEnd);
    
    registerCardWithUID(uid, name);
  }
  
  // Gate RFID scan mode control
  if (String(topic) == "iot/gate/rfid/scanmode") {
    if (message == "START") {
      isScanMode = true;
      Serial.println("Gate scan mode activated");
    } else if (message == "STOP") {
      isScanMode = false;
      Serial.println("Gate scan mode deactivated");
    }
  }
  
  // Gate RFID delete
  if (String(topic) == "iot/gate/rfid/delete") {
    deleteCard(message);
  }
}

void unlockDoor() {
  isDoorLocked = false;
  doorServo.write(90); // Open position
  Serial.println("Door UNLOCKED");
  beep(1); // 1 beep for success
  sendLogToESP4("Door:UNLOCKED");
  client.publish("iot/door/status", "UNLOCKED");
  client.publish("iot/esp32_3/status", "Door: UNLOCKED");
  
  // Auto lock after 5 seconds
  delay(5000);
  lockDoor();
}

void lockDoor() {
  isDoorLocked = true;
  doorServo.write(0); // Locked position
  Serial.println("Door LOCKED");
  sendLogToESP4("Door:LOCKED");
  client.publish("iot/door/status", "LOCKED");
  client.publish("iot/esp32_3/status", "Door: LOCKED");
}

void beep(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

String getCardUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

bool isCardRegistered(String uid) {
  for (int i = 0; i < cardCount; i++) {
    if (registeredCards[i] == uid) {
      return true;
    }
  }
  return false;
}

void registerCardWithUID(String uid, String name) {
  if (cardCount < 10) {
    registeredCards[cardCount] = uid;
    cardCount++;
    
    Serial.println("Card registered: " + uid + " - " + name);
    
    String data = "{\"uid\":\"" + uid + "\",\"name\":\"" + name + "\",\"addedAt\":\"" + String(millis()) + "\"}";
    client.publish("iot/gate/rfid/registered", data.c_str());
    client.publish("iot/esp32_3/status", "Card registered");
    
    beep(2);
    isScanMode = false;
  }
}

void deleteCard(String uid) {
  for (int i = 0; i < cardCount; i++) {
    if (registeredCards[i] == uid) {
      for (int j = i; j < cardCount - 1; j++) {
        registeredCards[j] = registeredCards[j + 1];
      }
      cardCount--;
      break;
    }
  }
}

void logAccess(String method, String status, String identifier) {
  String log = "{\"time\":\"" + String(millis()) + "\",\"method\":\"" + method + "\",\"status\":\"" + status + "\",\"identifier\":\"" + identifier + "\"}";
  client.publish("iot/door/access/log", log.c_str());
}

// Smart Door Functions (IR sensor only)
void checkDoorSensor() {
  int irValue = digitalRead(DOOR_IR_SENSOR_PIN);
  
  // IR sensor: LOW = person detected, HIGH = no person
  if (irValue == LOW && isDoorLocked) {
    Serial.println("Person detected - Opening door");
    unlockDoor(); // Auto unlock when person detected
  }
}

// Smart Gate Functions
void checkGateSensor() {
  int irValue = digitalRead(GATE_IR_SENSOR_PIN);
  
  // IR sensor: LOW = car detected from inside (EXIT), HIGH = no car
  if (irValue == LOW && !isGateOpen) {
    Serial.println("Car detected from inside - Opening gate for EXIT");
    sendLogToESP4("Car exiting");
    client.publish("iot/gate/status", "CAR_EXITING");
    client.publish("iot/esp32_3/status", "Car exiting");
    
    // Open gate immediately for exit (no RFID needed)
    openGate();
  }
}

void openGate() {
  if (isGateOpen) return;
  
  Serial.println("Opening gate...");
  sendLogToESP4("Gate opening");
  client.publish("iot/esp32_3/status", "Gate opening");
  
  // Open both servos
  gateServoLeft.write(90);   // Left servo: 0° to 90°
  gateServoRight.write(0);   // Right servo: 90° to 0° (reverse direction)
  
  isGateOpen = true;
  beep(2);
  
  client.publish("iot/gate/status", "OPEN");
  sendLogToESP4("Gate: OPEN");
  
  Serial.println("Gate opened!");
  
  // Auto close after 5 seconds
  delay(5000);
  closeGate();
}

void closeGate() {
  if (!isGateOpen && gateServoLeft.read() == 0) return; // Already closed
  
  Serial.println("Closing gate...");
  sendLogToESP4("Gate closing");
  client.publish("iot/esp32_3/status", "Gate closing");
  
  // Close both servos
  gateServoLeft.write(0);    // Left servo: back to 0°
  gateServoRight.write(90);  // Right servo: back to 90°
  
  isGateOpen = false;
  beep(1);
  
  client.publish("iot/gate/status", "CLOSED");
  sendLogToESP4("Gate: CLOSED");
  
  Serial.println("Gate closed!");
}

void reconnect() {
  // Only try to reconnect, don't block forever
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_3_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("iot/door/control");
      client.subscribe("iot/door/emergency");
      client.subscribe("iot/esp32_3/ping");
      
      // Subscribe to gate topics
      client.subscribe("iot/gate/control");
      client.subscribe("iot/gate/rfid/register");
      client.subscribe("iot/gate/rfid/scanmode");
      client.subscribe("iot/gate/rfid/delete");
      
      client.publish("iot/esp32_3/heartbeat", "ONLINE");
      Serial.println("✓ MQTT Connected & Heartbeat sent!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" - will retry in next loop");
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
  
  // Setup MQTT if not configured yet (after WiFi connects)
  static bool mqttConfigured = false;
  if (!mqttConfigured && WiFi.status() == WL_CONNECTED) {
    Serial.println("Setting up MQTT client...");
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    mqttConfigured = true;
    Serial.println("✓ MQTT client configured!");
  }
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long currentMillis = millis();
  
  // Check door sensor (IR only)
  if (currentMillis - lastDoorCheck >= doorCheckInterval) {
    lastDoorCheck = currentMillis;
    checkDoorSensor();
  }
  
  // Check gate sensor
  if (currentMillis - lastGateCheck >= gateCheckInterval) {
    lastGateCheck = currentMillis;
    checkGateSensor();
  }
  
  // Check for RFID card (Gate ENTRY only)
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = getCardUID();
    Serial.println("Card detected: " + uid);
    
    // RFID for gate entry
    if (isCardRegistered(uid)) {
      Serial.println("Access Granted - Opening Gate for ENTRY");
      sendLogToESP4("Gate entry OK");
      client.publish("iot/esp32_3/status", "Gate entry granted");
      
      String data = "{\"uid\":\"" + uid + "\",\"status\":\"GATE_ENTRY_GRANTED\"}";
      client.publish("iot/gate/access/log", data.c_str());
      
      openGate();
    } else {
      Serial.println("Access Denied - Unknown Card");
      sendLogToESP4("Gate entry denied");
      client.publish("iot/esp32_3/status", "Gate entry denied");
      beep(3); // 3 beeps for denied
      
      String data = "{\"uid\":\"" + uid + "\",\"status\":\"GATE_ENTRY_DENIED\"}";
      client.publish("iot/gate/access/log", data.c_str());
    }
    
    // If in scan mode (registering new card for gate)
    if (isScanMode) {
      Serial.println("Card scanned: " + uid);
      sendLogToESP4("Scanning...");
      
      String data = "{\"uid\":\"" + uid + "\",\"status\":\"SCANNED\"}";
      client.publish("iot/gate/rfid/scan", data.c_str());
      client.publish("iot/esp32_3/status", "Scanning card...");
      
      beep(1);
    }
    
    mfrc522.PICC_HaltA();
  }
  
  // Send heartbeat and status
  if (currentMillis - lastHeartbeat >= heartbeatInterval) {
    lastHeartbeat = currentMillis;
    client.publish("iot/esp32_3/heartbeat", "ONLINE");
    
    String status = "Door: ";
    status += isDoorLocked ? "LOCKED" : "UNLOCKED";
    client.publish("iot/esp32_3/status", status.c_str());
  }
}
