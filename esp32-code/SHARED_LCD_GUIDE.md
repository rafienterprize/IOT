# 📟 Cara Sharing 1 LCD untuk 3 ESP32

## 🎯 Konsep

```
ESP32 #1 (Lamp, Gas, Feeder)
    ↓ MQTT
ESP32 #2 (Trash, Clothesline) → MQTT Broker → ESP32 Master (LCD Controller)
    ↓ MQTT                                            ↓
ESP32 #3 (Smart Door)                            LCD Display
```

**ESP32 Master:**
- Kontrol LCD I2C
- Subscribe ke semua topics dari ESP32 lain
- Tampilkan info dari semua device

---

## 🔧 Solusi 1: ESP32 #3 (Smart Door) Jadi Master LCD

Karena ESP32 #3 sudah punya LCD untuk Smart Door, kita tambahkan fungsi untuk tampilkan info dari ESP32 lain juga.

### Modifikasi ESP32 #3 Code

#### 1. Tambah Subscribe Topics

```cpp
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_3_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      
      // Smart Door topics
      client.subscribe("iot/door/control");
      client.subscribe("iot/door/emergency");
      client.subscribe("iot/door/rfid/register");
      client.subscribe("iot/door/rfid/scanmode");
      client.subscribe("iot/door/rfid/delete");
      client.subscribe("iot/door/pin/register");
      client.subscribe("iot/door/pin/delete");
      client.subscribe("iot/esp32_3/wifi/config");
      
      // Subscribe to other ESP32 status
      client.subscribe("iot/lamp/status");
      client.subscribe("iot/gas/level");
      client.subscribe("iot/feeder/status");
      client.subscribe("iot/trash/organik/level");
      client.subscribe("iot/clothesline/status");
      client.subscribe("iot/clothesline/rain");
      
      client.publish("iot/esp32_3/heartbeat", "ONLINE");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
```

#### 2. Tambah Variables untuk Multi-Device Display

```cpp
// Variables for multi-device display
String lampStatus = "OFF";
int gasLevel = 0;
String feederStatus = "IDLE";
int trashLevel = 0;
String clotheslineStatus = "CLOSED";
bool isRaining = false;

unsigned long lastDisplayUpdate = 0;
const long displayInterval = 3000; // Ganti display setiap 3 detik
int displayMode = 0; // 0=Door, 1=Lamp/Gas, 2=Trash/Clothesline
```

#### 3. Update Callback untuk Handle Semua Topics

```cpp
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  
  // Smart Door topics (existing code)
  if (String(topic) == "iot/door/control") {
    // ... existing door control code
  }
  
  // ESP32 #1 topics
  if (String(topic) == "iot/lamp/status") {
    lampStatus = message;
  }
  
  if (String(topic) == "iot/gas/level") {
    gasLevel = message.toInt();
  }
  
  if (String(topic) == "iot/feeder/status") {
    feederStatus = message;
  }
  
  // ESP32 #2 topics
  if (String(topic) == "iot/trash/organik/level") {
    trashLevel = message.toInt();
  }
  
  if (String(topic) == "iot/clothesline/status") {
    clotheslineStatus = message;
  }
  
  if (String(topic) == "iot/clothesline/rain") {
    isRaining = (message == "RAIN");
  }
}
```

#### 4. Tambah Function untuk Rotating Display

```cpp
void updateMultiDeviceDisplay() {
  unsigned long currentMillis = millis();
  
  // Rotate display every 3 seconds
  if (currentMillis - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = currentMillis;
    displayMode = (displayMode + 1) % 3;
    
    lcd.clear();
    
    switch (displayMode) {
      case 0: // Smart Door (default)
        lcd.setCursor(0, 0);
        lcd.print("Smart Door");
        lcd.setCursor(0, 1);
        lcd.print(isDoorLocked ? "LOCKED" : "UNLOCKED");
        break;
        
      case 1: // ESP32 #1 Status
        lcd.setCursor(0, 0);
        lcd.print("Lamp:");
        lcd.print(lampStatus);
        lcd.setCursor(0, 1);
        lcd.print("Gas:");
        lcd.print(gasLevel);
        lcd.print("ppm");
        break;
        
      case 2: // ESP32 #2 Status
        lcd.setCursor(0, 0);
        lcd.print("Trash:");
        lcd.print(trashLevel);
        lcd.print("%");
        lcd.setCursor(0, 1);
        lcd.print(isRaining ? "Rain!" : "Clear");
        lcd.print(" ");
        lcd.print(clotheslineStatus);
        break;
    }
  }
}
```

#### 5. Update Loop

```cpp
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long currentMillis = millis();
  
  // ... existing RFID and keypad code ...
  
  // Update multi-device display
  if (!isScanMode && !isRegistering && enteredPin.length() == 0) {
    updateMultiDeviceDisplay();
  }
  
  // Send heartbeat
  if (currentMillis - lastHeartbeat >= heartbeatInterval) {
    lastHeartbeat = currentMillis;
    client.publish("iot/esp32_3/heartbeat", "ONLINE");
  }
}
```

---

## 🔧 Solusi 2: ESP32 Dedicated untuk LCD (Recommended)

Gunakan 1 ESP32 khusus untuk LCD yang hanya display info dari 3 ESP32 lainnya.

### ESP32 LCD Master Code

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "NAMA_WIFI_KAMU";
const char* password = "PASSWORD_WIFI_KAMU";

// MQTT Broker
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Device status
String lampStatus = "OFF";
int gasLevel = 0;
String feederStatus = "IDLE";
int trashOrganik = 0;
int trashAnorganik = 0;
int trashMetal = 0;
String clotheslineStatus = "CLOSED";
bool isRaining = false;
String doorStatus = "LOCKED";
bool esp32_1_online = false;
bool esp32_2_online = false;
bool esp32_3_online = false;

// Display control
unsigned long lastDisplayUpdate = 0;
const long displayInterval = 3000;
int displayMode = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("IoT Dashboard");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready!");
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // ESP32 #1 topics
  if (String(topic) == "iot/lamp/status") {
    lampStatus = message;
  }
  if (String(topic) == "iot/gas/level") {
    gasLevel = message.toFloat();
  }
  if (String(topic) == "iot/feeder/status") {
    feederStatus = message;
  }
  if (String(topic) == "iot/esp32_1/heartbeat") {
    esp32_1_online = true;
  }
  
  // ESP32 #2 topics
  if (String(topic) == "iot/trash/organik/level") {
    trashOrganik = message.toInt();
  }
  if (String(topic) == "iot/trash/anorganik/level") {
    trashAnorganik = message.toInt();
  }
  if (String(topic) == "iot/trash/metal/level") {
    trashMetal = message.toInt();
  }
  if (String(topic) == "iot/clothesline/status") {
    clotheslineStatus = message;
  }
  if (String(topic) == "iot/clothesline/rain") {
    isRaining = (message == "RAIN");
  }
  if (String(topic) == "iot/esp32_2/heartbeat") {
    esp32_2_online = true;
  }
  
  // ESP32 #3 topics
  if (String(topic) == "iot/door/status") {
    doorStatus = message;
  }
  if (String(topic) == "iot/esp32_3/heartbeat") {
    esp32_3_online = true;
  }
}

void updateDisplay() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = currentMillis;
    displayMode = (displayMode + 1) % 6;
    
    lcd.clear();
    
    switch (displayMode) {
      case 0: // System Status
        lcd.setCursor(0, 0);
        lcd.print("System Status");
        lcd.setCursor(0, 1);
        lcd.print(esp32_1_online ? "1" : "X");
        lcd.print(" ");
        lcd.print(esp32_2_online ? "2" : "X");
        lcd.print(" ");
        lcd.print(esp32_3_online ? "3" : "X");
        lcd.print(" Online");
        break;
        
      case 1: // Smart Lamp
        lcd.setCursor(0, 0);
        lcd.print("Smart Lamp");
        lcd.setCursor(0, 1);
        lcd.print("Status: ");
        lcd.print(lampStatus);
        break;
        
      case 2: // Gas Detector
        lcd.setCursor(0, 0);
        lcd.print("Gas Detector");
        lcd.setCursor(0, 1);
        lcd.print(gasLevel);
        lcd.print(" PPM");
        if (gasLevel > 500) {
          lcd.print(" DANGER!");
        }
        break;
        
      case 3: // Smart Trash
        lcd.setCursor(0, 0);
        lcd.print("Trash Bins");
        lcd.setCursor(0, 1);
        lcd.print("O:");
        lcd.print(trashOrganik);
        lcd.print(" A:");
        lcd.print(trashAnorganik);
        lcd.print(" M:");
        lcd.print(trashMetal);
        break;
        
      case 4: // Clothesline
        lcd.setCursor(0, 0);
        lcd.print("Clothesline");
        lcd.setCursor(0, 1);
        lcd.print(clotheslineStatus);
        lcd.print(" ");
        lcd.print(isRaining ? "RAIN!" : "Clear");
        break;
        
      case 5: // Smart Door
        lcd.setCursor(0, 0);
        lcd.print("Smart Door");
        lcd.setCursor(0, 1);
        lcd.print(doorStatus);
        break;
    }
    
    // Reset online status (will be set again by heartbeat)
    esp32_1_online = false;
    esp32_2_online = false;
    esp32_3_online = false;
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_LCD_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      
      // Subscribe to all topics
      client.subscribe("iot/lamp/status");
      client.subscribe("iot/gas/level");
      client.subscribe("iot/feeder/status");
      client.subscribe("iot/trash/organik/level");
      client.subscribe("iot/trash/anorganik/level");
      client.subscribe("iot/trash/metal/level");
      client.subscribe("iot/clothesline/status");
      client.subscribe("iot/clothesline/rain");
      client.subscribe("iot/door/status");
      client.subscribe("iot/esp32_1/heartbeat");
      client.subscribe("iot/esp32_2/heartbeat");
      client.subscribe("iot/esp32_3/heartbeat");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  updateDisplay();
}
```

---

## 🔧 Solusi 3: I2C Bus Sharing (Hardware)

Secara hardware, bisa share 1 LCD ke 3 ESP32 via I2C bus, tapi **TIDAK RECOMMENDED** karena:
- Conflict jika 2 ESP32 nulis bersamaan
- Butuh arbitration logic yang kompleks
- Lebih baik pakai MQTT

---

## 📊 Perbandingan Solusi

| Solusi | Kelebihan | Kekurangan |
|--------|-----------|------------|
| **ESP32 #3 Jadi Master** | Hemat 1 ESP32 | LCD sibuk untuk door, info lain cuma sebentar |
| **ESP32 Dedicated LCD** | Display optimal, tidak ganggu device lain | Butuh 1 ESP32 tambahan |
| **I2C Sharing** | Hemat ESP32 | Kompleks, conflict, tidak recommended |

---

## 🎯 Rekomendasi

### Untuk Project Kamu:

**Gunakan Solusi 1: ESP32 #3 Jadi Master**

Karena:
- ✅ Tidak perlu ESP32 tambahan
- ✅ LCD sudah ada di ESP32 #3
- ✅ Cukup update code
- ✅ Display rotate setiap 3 detik

**Display Rotation:**
```
0-3 detik: Smart Door (LOCKED/UNLOCKED)
3-6 detik: Lamp (ON/OFF) + Gas (XXX ppm)
6-9 detik: Trash (XX%) + Clothesline (OPEN/CLOSED)
... repeat
```

### Untuk Production/Demo:

**Gunakan Solusi 2: ESP32 Dedicated LCD**

Karena:
- ✅ Display lebih jelas dan lengkap
- ✅ Tidak ganggu fungsi Smart Door
- ✅ Bisa tampilkan lebih banyak info
- ✅ Lebih profesional

---

## 🚀 Implementasi

### Quick Start (Solusi 1):

1. Backup ESP32 #3 code
2. Copy code modifikasi di atas
3. Upload ke ESP32 #3
4. LCD akan rotate display setiap 3 detik
5. Done!

### Quick Start (Solusi 2):

1. Siapkan 1 ESP32 tambahan
2. Pasang LCD I2C ke ESP32 baru
3. Upload code ESP32 LCD Master
4. LCD akan tampilkan semua device info
5. Done!

---

## 💡 Tips

1. **Adjust Display Interval**: Ubah `displayInterval` sesuai kebutuhan (2-5 detik)
2. **Prioritas Display**: Ubah urutan `displayMode` untuk prioritas info
3. **Alert Mode**: Tambah logic untuk tampilkan alert (gas tinggi, trash penuh) lebih lama
4. **Manual Switch**: Tambah button untuk manual switch display mode

---

Pilih solusi yang sesuai kebutuhan kamu bro! Kalau mau hemat ESP32, pakai Solusi 1. Kalau mau display optimal, pakai Solusi 2. 🚀
