#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
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

// RFID RC522 Pins
#define RST_PIN 22
#define SS_PIN 21

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Door Lock Pin
#define LOCK_PIN 13
#define BUZZER_PIN 14

// Keypad 4x4
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {32, 33, 25, 26};
byte colPins[COLS] = {27, 14, 12, 13};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variables
bool isDoorLocked = true;
String registeredCards[10];
int cardCount = 0;
String registeredPins[10];
int pinCount = 0;
String enteredPin = "";
bool isScanMode = false;

unsigned long lastHeartbeat = 0;
const long heartbeatInterval = 10000;

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
  EEPROM.begin(EEPROM_SIZE);
  
  // Initialize Serial with ESP32 #4
  SerialToESP4.begin(9600, SERIAL_8N1, RX_FROM_ESP4, TX_TO_ESP4); // RX=16, TX=17
  
  // Initialize SPI and RFID
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Initialize pins
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, HIGH); // Locked
  
  // Try to connect to WiFi
  if (connectToWiFi()) {
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
  } else {
    Serial.println("Waiting for WiFi config from ESP32 #4...");
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
  
  // RFID registration
  if (String(topic) == "iot/door/rfid/register") {
    int uidStart = message.indexOf("\"uid\":\"") + 7;
    int uidEnd = message.indexOf("\"", uidStart);
    String uid = message.substring(uidStart, uidEnd);
    
    int nameStart = message.indexOf("\"name\":\"") + 8;
    int nameEnd = message.indexOf("\"", nameStart);
    String name = message.substring(nameStart, nameEnd);
    
    registerCardWithUID(uid, name);
  }
  
  // Scan mode control
  if (String(topic) == "iot/door/rfid/scanmode") {
    if (message == "START") {
      isScanMode = true;
      Serial.println("Scan mode activated");
    } else if (message == "STOP") {
      isScanMode = false;
      Serial.println("Scan mode deactivated");
    }
  }
  
  // RFID delete
  if (String(topic) == "iot/door/rfid/delete") {
    deleteCard(message);
  }
  
  // PIN registration
  if (String(topic) == "iot/door/pin/register") {
    int pinStart = message.indexOf("\"pin\":\"") + 7;
    int pinEnd = message.indexOf("\"", pinStart);
    String pin = message.substring(pinStart, pinEnd);
    registerPin(pin);
  }
  
  // PIN delete
  if (String(topic) == "iot/door/pin/delete") {
    deletePin(message);
  }
  
  // Ping response
  if (String(topic) == "iot/esp32_3/ping") {
    if (message == "PING") {
      client.publish("iot/esp32_3/pong", "PONG");
    }
  }
}

void lockDoor() {
  isDoorLocked = true;
  digitalWrite(LOCK_PIN, HIGH);
  Serial.println("Door LOCKED");
  beep(1);
  sendLogToESP4("Door:LOCKED");
  client.publish("iot/door/status", "LOCKED");
  client.publish("iot/esp32_3/status", "Door: LOCKED");
}

void unlockDoor() {
  isDoorLocked = false;
  digitalWrite(LOCK_PIN, LOW);
  Serial.println("Door UNLOCKED - Welcome!");
  beep(2);
  sendLogToESP4("Door:UNLOCKED");
  client.publish("iot/door/status", "UNLOCKED");
  client.publish("iot/esp32_3/status", "Door: UNLOCKED");
  
  // Auto lock after 5 seconds
  delay(5000);
  lockDoor();
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
    client.publish("iot/door/rfid/registered", data.c_str());
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

void registerPin(String pin) {
  if (pinCount < 10) {
    registeredPins[pinCount] = pin;
    pinCount++;
    Serial.println("PIN registered: " + pin);
  }
}

void deletePin(String pin) {
  for (int i = 0; i < pinCount; i++) {
    if (registeredPins[i] == pin) {
      for (int j = i; j < pinCount - 1; j++) {
        registeredPins[j] = registeredPins[j + 1];
      }
      pinCount--;
      break;
    }
  }
}

bool isPinRegistered(String pin) {
  for (int i = 0; i < pinCount; i++) {
    if (registeredPins[i] == pin) {
      return true;
    }
  }
  return false;
}

void logAccess(String method, String status, String identifier) {
  String log = "{\"time\":\"" + String(millis()) + "\",\"method\":\"" + method + "\",\"status\":\"" + status + "\",\"identifier\":\"" + identifier + "\"}";
  client.publish("iot/door/access/log", log.c_str());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_3_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("iot/door/control");
      client.subscribe("iot/door/emergency");
      client.subscribe("iot/door/rfid/register");
      client.subscribe("iot/door/rfid/scanmode");
      client.subscribe("iot/door/rfid/delete");
      client.subscribe("iot/door/pin/register");
      client.subscribe("iot/door/pin/delete");
      client.subscribe("iot/esp32_3/ping");
      
      client.publish("iot/esp32_3/heartbeat", "ONLINE");
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
  
  // Check for RFID card
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = getCardUID();
    Serial.println("Card detected: " + uid);
    
    if (isScanMode) {
      Serial.println("Card scanned in scan mode: " + uid);
      sendLogToESP4("Scanning...");
      
      String data = "{\"uid\":\"" + uid + "\",\"status\":\"SCANNED\"}";
      client.publish("iot/door/rfid/scan", data.c_str());
      client.publish("iot/esp32_3/status", "Scanning card...");
      
      beep(1);
    } else {
      if (isCardRegistered(uid)) {
        Serial.println("Access Granted!");
        sendLogToESP4("Access Granted");
        client.publish("iot/esp32_3/status", "Access Granted");
        unlockDoor();
        logAccess("RFID", "GRANTED", uid);
        
        String data = "{\"uid\":\"" + uid + "\",\"status\":\"ACCESS_GRANTED\"}";
        client.publish("iot/door/rfid/scan", data.c_str());
      } else {
        Serial.println("Access Denied - Unknown Card");
        sendLogToESP4("Access Denied");
        client.publish("iot/esp32_3/status", "Access Denied");
        beep(3);
        logAccess("RFID", "DENIED", uid);
        delay(2000);
        client.publish("iot/esp32_3/status", "Door: LOCKED");
      }
    }
    
    mfrc522.PICC_HaltA();
  }
  
  // Check for keypad input
  char key = keypad.getKey();
  if (key) {
    Serial.println("Key pressed: " + String(key));
    
    if (key == '#') {
      if (enteredPin.length() == 4) {
        if (isPinRegistered(enteredPin)) {
          Serial.println("PIN Correct");
          unlockDoor();
          logAccess("PIN", "GRANTED", enteredPin);
          
          String data = "{\"pin\":\"" + enteredPin + "\",\"status\":\"ACCESS_GRANTED\"}";
          client.publish("iot/door/pin/entry", data.c_str());
        } else {
          Serial.println("Wrong PIN");
          beep(3);
          logAccess("PIN", "DENIED", enteredPin);
          delay(2000);
        }
      }
      enteredPin = "";
    } else if (key == '*') {
      enteredPin = "";
      Serial.println("PIN Cleared");
    } else if (key >= '0' && key <= '9') {
      if (enteredPin.length() < 4) {
        enteredPin += key;
        Serial.println("PIN: " + String(enteredPin.length()) + " digits entered");
      }
    }
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
