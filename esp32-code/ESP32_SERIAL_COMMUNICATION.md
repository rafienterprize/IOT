# ESP32 Serial Communication Guide

Panduan komunikasi serial antar ESP32 untuk display LCD real-time tanpa delay MQTT.

---

## Opsi 1: UART Serial Communication (Recommended)

### Wiring Diagram

```
ESP32 #1                    ESP32 #3 (Master LCD)
├── TX2 (GPIO 17) ────────→ RX2 (GPIO 16)
└── GND ──────────────────→ GND

ESP32 #2                    ESP32 #3 (Master LCD)
├── TX2 (GPIO 17) ────────→ RX1 (GPIO 4)
└── GND ──────────────────→ GND
```

### Pin Configuration

**ESP32 #1 (Sender)**
- TX2: GPIO 17 → Kirim data ke ESP32 #3

**ESP32 #2 (Sender)**
- TX2: GPIO 17 → Kirim data ke ESP32 #3

**ESP32 #3 (Master LCD - Receiver)**
- RX2: GPIO 16 → Terima data dari ESP32 #1
- RX1: GPIO 4 → Terima data dari ESP32 #2

### Code Implementation

#### ESP32 #1 - Sender Code

```cpp
// Add to ESP32_1_Lamp_Gas_Feeder.ino

// Serial communication to ESP32 #3
#define TX_PIN 17
HardwareSerial SerialToLCD(2); // Use Serial2

void setup() {
  // ... existing setup code ...
  
  // Initialize Serial to LCD
  SerialToLCD.begin(9600, SERIAL_8N1, -1, TX_PIN); // RX=-1 (not used), TX=17
}

void sendToLCD(String message) {
  // Send data to ESP32 #3 via serial
  SerialToLCD.print("ESP1:");
  SerialToLCD.println(message);
}

void loop() {
  // ... existing loop code ...
  
  // Send status to LCD every time it changes
  if (lampState changed) {
    String status = "Lamp: ";
    status += lampState ? "ON" : "OFF";
    sendToLCD(status);
  }
  
  if (gasLevel > 500) {
    sendToLCD("Gas: DANGER!");
  }
}
```

#### ESP32 #2 - Sender Code

```cpp
// Add to ESP32_2_Trash_Clothesline_Updated.ino

// Serial communication to ESP32 #3
#define TX_PIN 17
HardwareSerial SerialToLCD(2); // Use Serial2

void setup() {
  // ... existing setup code ...
  
  // Initialize Serial to LCD
  SerialToLCD.begin(9600, SERIAL_8N1, -1, TX_PIN); // RX=-1 (not used), TX=17
}

void sendToLCD(String message) {
  // Send data to ESP32 #3 via serial
  SerialToLCD.print("ESP2:");
  SerialToLCD.println(message);
}

void loop() {
  // ... existing loop code ...
  
  // Send status to LCD
  if (clotheslineOpen changed) {
    String status = "Clothesline: ";
    status += clotheslineOpen ? "OPEN" : "CLOSED";
    sendToLCD(status);
  }
  
  if (isRaining) {
    sendToLCD("Rain Detected!");
  }
}
```

#### ESP32 #3 - Master LCD Receiver Code

```cpp
// Add to ESP32_3_Smart_Door.ino

// Serial communication from other ESP32s
#define RX1_PIN 16  // From ESP32 #1
#define RX2_PIN 4   // From ESP32 #2

HardwareSerial SerialFromESP1(1); // Serial1 for ESP32 #1
HardwareSerial SerialFromESP2(2); // Serial2 for ESP32 #2

String esp1Status = "Idle";
String esp2Status = "Idle";

void setup() {
  // ... existing setup code ...
  
  // Initialize Serial receivers
  SerialFromESP1.begin(9600, SERIAL_8N1, RX1_PIN, -1); // RX=16, TX=-1 (not used)
  SerialFromESP2.begin(9600, SERIAL_8N1, RX2_PIN, -1); // RX=4, TX=-1 (not used)
}

void loop() {
  // ... existing loop code ...
  
  // Read from ESP32 #1
  if (SerialFromESP1.available()) {
    String data = SerialFromESP1.readStringUntil('\n');
    if (data.startsWith("ESP1:")) {
      esp1Status = data.substring(5); // Remove "ESP1:" prefix
    }
  }
  
  // Read from ESP32 #2
  if (SerialFromESP2.available()) {
    String data = SerialFromESP2.readStringUntil('\n');
    if (data.startsWith("ESP2:")) {
      esp2Status = data.substring(5); // Remove "ESP2:" prefix
    }
  }
  
  // Update LCD display with received data
  updateMultiDeviceDisplay();
}

void updateMultiDeviceDisplay() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = currentMillis;
    displayMode = (displayMode + 1) % 3;
    
    lcd.clear();
    
    switch (displayMode) {
      case 0: // Smart Door (local)
        lcd.setCursor(0, 0);
        lcd.print("Smart Door");
        lcd.setCursor(0, 1);
        lcd.print(isDoorLocked ? "LOCKED" : "UNLOCKED");
        break;
        
      case 1: // ESP32 #1 Status (via Serial)
        lcd.setCursor(0, 0);
        lcd.print("ESP32 1");
        lcd.setCursor(0, 1);
        lcd.print(esp1Status);
        break;
        
      case 2: // ESP32 #2 Status (via Serial)
        lcd.setCursor(0, 0);
        lcd.print("ESP32 2");
        lcd.setCursor(0, 1);
        lcd.print(esp2Status);
        break;
    }
  }
}
```

---

## Opsi 2: I2C Communication (Alternative)

Jika ingin lebih banyak ESP32 terhubung ke 1 bus.

### Wiring Diagram

```
ESP32 #1                    ESP32 #3 (Master)
├── SDA (GPIO 21) ─────────→ SDA (GPIO 21)
├── SCL (GPIO 22) ─────────→ SCL (GPIO 22)
└── GND ───────────────────→ GND

ESP32 #2                    ESP32 #3 (Master)
├── SDA (GPIO 21) ─────────→ SDA (GPIO 21)
├── SCL (GPIO 22) ─────────→ SCL (GPIO 22)
└── GND ───────────────────→ GND
```

**Note:** I2C adalah bus, jadi semua device share SDA dan SCL yang sama.

### Code Implementation

#### ESP32 #1 & #2 - I2C Slave

```cpp
#include <Wire.h>

#define I2C_SLAVE_ADDR_ESP1 0x08  // ESP32 #1 address
#define I2C_SLAVE_ADDR_ESP2 0x09  // ESP32 #2 address

String statusMessage = "Idle";

void setup() {
  Wire.begin(I2C_SLAVE_ADDR_ESP1); // or ESP2
  Wire.onRequest(requestEvent);
}

void requestEvent() {
  Wire.write(statusMessage.c_str());
}

void loop() {
  // Update statusMessage based on device state
  statusMessage = "Lamp: ON";
}
```

#### ESP32 #3 - I2C Master

```cpp
#include <Wire.h>

#define I2C_SLAVE_ADDR_ESP1 0x08
#define I2C_SLAVE_ADDR_ESP2 0x09

String esp1Status = "Idle";
String esp2Status = "Idle";

void setup() {
  Wire.begin(); // Master mode
}

void loop() {
  // Request data from ESP32 #1
  Wire.requestFrom(I2C_SLAVE_ADDR_ESP1, 16);
  if (Wire.available()) {
    esp1Status = "";
    while (Wire.available()) {
      esp1Status += (char)Wire.read();
    }
  }
  
  // Request data from ESP32 #2
  Wire.requestFrom(I2C_SLAVE_ADDR_ESP2, 16);
  if (Wire.available()) {
    esp2Status = "";
    while (Wire.available()) {
      esp2Status += (char)Wire.read();
    }
  }
  
  // Update LCD
  updateMultiDeviceDisplay();
  
  delay(100);
}
```

---

## Perbandingan Metode

| Metode | Latency | Kabel | Kompleksitas | Jarak Max |
|--------|---------|-------|--------------|-----------|
| **MQTT WiFi** | 50-200ms | ❌ Tidak | Rendah | Unlimited (WiFi range) |
| **UART Serial** | <1ms | ✅ Ya (2 kabel per ESP32) | Sedang | 5-10 meter |
| **I2C** | <1ms | ✅ Ya (2 kabel shared) | Sedang | 1-2 meter |

---

## Rekomendasi

### Gunakan MQTT WiFi jika:
- ✅ ESP32 berjauhan (>2 meter)
- ✅ Tidak mau ribet kabel
- ✅ Latency 50-200ms masih acceptable
- ✅ Mau akses dari website juga

### Gunakan UART Serial jika:
- ✅ Butuh latency <1ms (real-time)
- ✅ ESP32 berdekatan (<5 meter)
- ✅ Komunikasi point-to-point
- ✅ Tidak masalah dengan kabel

### Gunakan I2C jika:
- ✅ Butuh latency <1ms
- ✅ ESP32 sangat dekat (<1 meter)
- ✅ Mau hemat kabel (shared bus)
- ✅ Banyak device dalam 1 bus

---

## Kesimpulan

Untuk project IoT monitoring dengan website, **MQTT WiFi sudah optimal**. Latency 50-200ms tidak terasa untuk display LCD yang update setiap 3 detik.

Tapi jika benar-benar butuh real-time (<1ms), gunakan **UART Serial** dengan wiring di atas.

**Hybrid Solution (Best of Both Worlds):**
- Gunakan UART Serial untuk komunikasi ESP32 → LCD (real-time)
- Tetap gunakan MQTT WiFi untuk komunikasi ESP32 → Website (monitoring)

Dengan cara ini, LCD update instant tanpa delay, dan website tetap bisa monitor!
