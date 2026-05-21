# ESP32 #3 Servo Troubleshooting Guide

## Masalah: Servo Tidak Berputar

### Kemungkinan Penyebab & Solusi:

## 1. **Power Supply Tidak Cukup**
**Gejala**: Servo tidak bergerak sama sekali atau bergerak lemah
**Penyebab**: ESP32 hanya bisa supply 3.3V/5V dengan arus terbatas
**Solusi**:
```
- Gunakan external power supply 5V 2A untuk servo
- Hubungkan:
  * Servo VCC (merah) → External 5V
  * Servo GND (hitam/coklat) → Common GND (ESP32 + External PSU)
  * Servo Signal (kuning/orange) → ESP32 GPIO
```

## 2. **Pin GPIO Salah atau Konflik**
**Gejala**: Servo tidak respon sama sekali
**Cek Pin yang Digunakan**:
```cpp
#define DOOR_SERVO_PIN 13        // Door servo
#define GATE_SERVO_LEFT_PIN 25   // Gate left servo  
#define GATE_SERVO_RIGHT_PIN 26  // Gate right servo
```

**Pastikan**:
- Pin tidak digunakan komponen lain
- Pin bukan input-only (GPIO 34, 35, 36, 39)
- Pin bukan reserved (GPIO 6-11 untuk flash)

## 3. **Library ESP32Servo Tidak Terinstall**
**Gejala**: Compile error atau servo tidak attach
**Solusi**:
```
1. Arduino IDE → Tools → Manage Libraries
2. Search "ESP32Servo"
3. Install "ESP32Servo by Kevin Harrington"
4. Restart Arduino IDE
```

## 4. **Servo Rusak atau Kabel Putus**
**Test Servo Manual**:
```cpp
// Test code - upload ini untuk test servo
#include <ESP32Servo.h>

Servo testServo;

void setup() {
  Serial.begin(115200);
  testServo.attach(13); // Ganti dengan pin servo
  Serial.println("Testing servo...");
}

void loop() {
  Serial.println("Moving to 0°");
  testServo.write(0);
  delay(2000);
  
  Serial.println("Moving to 90°");
  testServo.write(90);
  delay(2000);
  
  Serial.println("Moving to 180°");
  testServo.write(180);
  delay(2000);
}
```

## 5. **MQTT Tidak Terkoneksi**
**Gejala**: Servo tidak respon dari website tapi bisa manual
**Cek Serial Monitor**:
```
✓ MQTT Connected & Heartbeat sent!  ← Harus ada ini
```

**Jika tidak ada**:
- Cek WiFi connection
- Cek broker MQTT (broker.hivemq.com)
- Restart ESP32

## 6. **Website Tidak Kirim Command**
**Debug MQTT Topics**:
```cpp
// Tambahkan di callback() untuk debug
Serial.println("Topic: " + String(topic));
Serial.println("Message: " + message);
```

**Topics yang harus ada**:
- `iot/door/control` → UNLOCK/LOCK
- `iot/gate/control` → OPEN/CLOSE

## 7. **Servo Angle Salah**
**Servo SG90**: 0° - 180°
**Servo MG996R**: 0° - 180°

**Jika servo bergerak tapi salah arah**:
```cpp
// Tukar nilai
doorServo.write(180 - angle); // Reverse direction
```

## 8. **Delay Terlalu Pendek**
**Masalah**: Servo belum sempat bergerak
**Solusi**: Tambah delay setelah write()
```cpp
doorServo.write(90);
delay(500); // Beri waktu servo bergerak
```

## Debug Steps:

### Step 1: Upload Debug Code
Upload kode ESP32 #3 yang sudah saya update dengan debug info. Cek Serial Monitor untuk:
```
Testing door servo movement...
Moving to 0 degrees (locked)...
Current position: 0
Moving to 90 degrees (unlocked)...
Current position: 90
```

### Step 2: Test Manual Command
Buka Serial Monitor, ketik:
```
AT+SERVO_TEST_DOOR
AT+SERVO_TEST_GATE
```

### Step 3: Test dari Website
1. Buka website
2. Klik "Unlock Door" 
3. Cek Serial Monitor untuk:
```
Message arrived [iot/door/control]: UNLOCK
UNLOCKING DOOR...
Moving servo to 90 degrees...
```

### Step 4: Cek Hardware
1. **Voltmeter**: Ukur tegangan servo (harus 5V)
2. **Multimeter**: Cek kontinuitas kabel
3. **Swap Servo**: Tukar dengan servo yang pasti jalan

## Wiring Diagram Benar:

```
ESP32 #3          Servo Door        Servo Gate L      Servo Gate R
GPIO 13    ────→  Signal (Orange)
GPIO 25    ─────────────────────→  Signal (Orange)
GPIO 26    ───────────────────────────────────────→  Signal (Orange)

External 5V ────→  VCC (Red)  ────→  VCC (Red)  ────→  VCC (Red)
ESP32 GND  ─────→  GND (Black) ───→  GND (Black) ───→  GND (Black)
```

## Quick Fix Commands:

### Reset Servo Position
```cpp
// Tambahkan di setup() untuk reset
doorServo.write(0);
delay(1000);
gateServoLeft.write(0);
gateServoRight.write(90);
delay(1000);
```

### Force Servo Movement
```cpp
// Tambahkan di loop() untuk test kontinyu
static unsigned long lastTest = 0;
if (millis() - lastTest > 5000) {
  doorServo.write(isDoorLocked ? 90 : 0);
  lastTest = millis();
}
```

Coba langkah-langkah ini dan lihat output Serial Monitor untuk debug lebih lanjut!