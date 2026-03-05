# ESP32 Pin Mapping Guide

Panduan lengkap koneksi pin untuk semua 3 ESP32 dalam sistem Smart Home IoT.

---

## ESP32 #1 - Lamp, Gas Detector, Fish Feeder

### Pin Configuration

| Komponen | Pin ESP32 | Keterangan |
|----------|-----------|------------|
| **Smart Lamp** | GPIO 2 | Output - Relay untuk lampu |
| **Gas Sensor MQ-2** | GPIO 34 | Input Analog - Sensor gas |
| **Fish Feeder Servo** | GPIO 18 | Output PWM - Servo motor |

### Wiring Diagram

```
ESP32 #1
├── GPIO 2  → Relay Module IN → Lampu 220V
├── GPIO 34 → MQ-2 Analog Out (A0)
├── GPIO 18 → Servo Signal (Orange/Yellow wire)
├── 5V      → Relay VCC, Servo VCC (Red wire)
├── 3.3V    → MQ-2 VCC
└── GND     → Relay GND, MQ-2 GND, Servo GND (Brown/Black wire)
```

### Komponen Details

**1. Smart Lamp (Relay)**
- Relay Module 1 Channel 5V
- Input: GPIO 2 (HIGH = OFF, LOW = ON)
- Load: Lampu 220V AC

**2. Gas Detector (MQ-2)**
- Sensor Gas MQ-2
- Output: Analog 0-4095 (12-bit ADC)
- Threshold: >500 PPM = Berbahaya

**3. Fish Feeder (Servo)**
- Servo SG90 atau MG996R
- Signal: GPIO 18 (PWM)
- Angle: 0° (tutup) → 90° (buka)

---

## ESP32 #2 - Smart Trash Bin, Smart Clothesline

### Pin Configuration

| Komponen | Pin ESP32 | Keterangan |
|----------|-----------|------------|
| **Rotation Servo** | GPIO 18 | Output PWM - Servo putar platform |
| **Gate Servo** | GPIO 19 | Output PWM - Servo pintu sampah |
| **TCS3200 S0** | GPIO 32 | Output - Frequency scaling |
| **TCS3200 S1** | GPIO 33 | Output - Frequency scaling |
| **TCS3200 S2** | GPIO 25 | Output - Color filter select |
| **TCS3200 S3** | GPIO 26 | Output - Color filter select |
| **TCS3200 OUT** | GPIO 27 | Input - Frequency output |
| **Metal Detector** | GPIO 14 | Input Digital - Sensor logam |
| **Ultrasonic Organik TRIG** | GPIO 12 | Output - Trigger pulse |
| **Ultrasonic Organik ECHO** | GPIO 13 | Input - Echo pulse |
| **Ultrasonic Anorganik TRIG** | GPIO 15 | Output - Trigger pulse |
| **Ultrasonic Anorganik ECHO** | GPIO 2 | Input - Echo pulse |
| **Ultrasonic Metal TRIG** | GPIO 4 | Output - Trigger pulse |
| **Ultrasonic Metal ECHO** | GPIO 5 | Input - Echo pulse |
| **Rain Sensor** | GPIO 34 | Input Analog - Sensor hujan |
| **Clothesline Motor** | GPIO 23 | Output - Motor jemuran |

### Wiring Diagram

```
ESP32 #2
├── GPIO 18 → Rotation Servo Signal
├── GPIO 19 → Gate Servo Signal
├── GPIO 32 → TCS3200 S0
├── GPIO 33 → TCS3200 S1
├── GPIO 25 → TCS3200 S2
├── GPIO 26 → TCS3200 S3
├── GPIO 27 → TCS3200 OUT
├── GPIO 14 → Metal Detector OUT
├── GPIO 12 → Ultrasonic 1 TRIG (Organik)
├── GPIO 13 → Ultrasonic 1 ECHO (Organik)
├── GPIO 15 → Ultrasonic 2 TRIG (Anorganik)
├── GPIO 2  → Ultrasonic 2 ECHO (Anorganik)
├── GPIO 4  → Ultrasonic 3 TRIG (Metal)
├── GPIO 5  → Ultrasonic 3 ECHO (Metal)
├── GPIO 34 → Rain Sensor Analog Out
├── GPIO 23 → Clothesline Motor Driver IN1
├── 5V      → All Servos VCC, Sensors VCC
└── GND     → All GND
```

### Komponen Details

**1. Smart Trash Bin**
- 2x Servo Motor (SG90 atau MG996R)
  - Rotation Servo: Putar platform 0°/120°/240°
  - Gate Servo: Buka/tutup pintu sampah
- TCS3200 Color Sensor: Deteksi warna sampah
- Metal Detector (Inductive Proximity Sensor): Deteksi logam
- 3x Ultrasonic HC-SR04: Monitor level sampah per bin

**2. Smart Clothesline**
- Rain Sensor (Analog): Deteksi hujan
- DC Motor + Driver L298N: Buka/tutup jemuran
- Threshold: <500 = Hujan, >500 = Cerah

---

## ESP32 #3 - Smart Door Lock (Master LCD)

### Pin Configuration

| Komponen | Pin ESP32 | Keterangan |
|----------|-----------|------------|
| **RFID RC522 RST** | GPIO 22 | Output - Reset pin |
| **RFID RC522 SS/SDA** | GPIO 21 | Output - Chip select |
| **RFID RC522 MOSI** | GPIO 23 | Output - SPI data out |
| **RFID RC522 MISO** | GPIO 19 | Input - SPI data in |
| **RFID RC522 SCK** | GPIO 18 | Output - SPI clock |
| **LCD I2C SDA** | GPIO 21 | I2C Data |
| **LCD I2C SCL** | GPIO 22 | I2C Clock |
| **Solenoid Lock** | GPIO 13 | Output - Relay untuk kunci |
| **Buzzer** | GPIO 14 | Output - Buzzer aktif |
| **Keypad Row 1** | GPIO 32 | Input - Baris 1 keypad |
| **Keypad Row 2** | GPIO 33 | Input - Baris 2 keypad |
| **Keypad Row 3** | GPIO 25 | Input - Baris 3 keypad |
| **Keypad Row 4** | GPIO 26 | Input - Baris 4 keypad |
| **Keypad Col 1** | GPIO 27 | Output - Kolom 1 keypad |
| **Keypad Col 2** | GPIO 14 | Output - Kolom 2 keypad |
| **Keypad Col 3** | GPIO 12 | Output - Kolom 3 keypad |
| **Keypad Col 4** | GPIO 13 | Output - Kolom 4 keypad |

### Wiring Diagram

```
ESP32 #3 (Master LCD)
├── SPI Bus (RFID RC522)
│   ├── GPIO 23 → MOSI
│   ├── GPIO 19 → MISO
│   ├── GPIO 18 → SCK
│   ├── GPIO 21 → SS/SDA
│   └── GPIO 22 → RST
│
├── I2C Bus (LCD 16x2)
│   ├── GPIO 21 → SDA (shared with RFID SS)
│   └── GPIO 22 → SCL (shared with RFID RST)
│
├── Digital Outputs
│   ├── GPIO 13 → Solenoid Lock Relay
│   └── GPIO 14 → Buzzer
│
└── Keypad 4x4 Matrix
    ├── Rows: GPIO 32, 33, 25, 26
    └── Cols: GPIO 27, 14, 12, 13
```

### Komponen Details

**1. RFID RC522**
- Module RFID RC522 (13.56MHz)
- Interface: SPI
- Baca kartu RFID/NFC untuk akses

**2. LCD 16x2 I2C**
- LCD 16x2 dengan modul I2C
- Address: 0x27 atau 0x3F
- Backlight: Biru
- Display: Intro animation, status pintu, rotasi info ESP32 lain

**3. Solenoid Lock**
- Solenoid Door Lock 12V
- Dikontrol via Relay Module
- HIGH = Locked, LOW = Unlocked

**4. Buzzer**
- Buzzer aktif 5V
- Beep saat: Intro (3+2 beep), akses granted (2 beep), denied (3 beep)

**5. Keypad 4x4**
- Matrix Keypad 4x4
- Layout:
  ```
  1 2 3 A
  4 5 6 B
  7 8 9 C
  * 0 # D
  ```
- Input PIN 4 digit, tekan # untuk submit, * untuk clear

---

## Power Supply Recommendations

### ESP32 #1
- ESP32: 5V 1A (via USB atau adapter)
- Relay: 5V (dari ESP32)
- MQ-2: 3.3V (dari ESP32)
- Servo: 5V 1A external (jika servo besar)

### ESP32 #2
- ESP32: 5V 2A (banyak komponen)
- 2x Servo: 5V 2A external power supply
- TCS3200: 5V (dari ESP32)
- Motor: 12V 2A (via L298N driver)
- Sensors: 5V (dari ESP32)

### ESP32 #3
- ESP32: 5V 1A
- RFID: 3.3V (dari ESP32)
- LCD: 5V (dari ESP32)
- Solenoid: 12V 1A (via relay + external PSU)
- Buzzer: 5V (dari ESP32)

**PENTING**: Gunakan power supply terpisah untuk motor dan solenoid! Jangan bebankan ke ESP32 langsung.

---

## I2C Address Detection

Jika LCD tidak muncul teks, scan alamat I2C dengan kode ini:

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("Scanning I2C...");
  
  for(byte i = 0; i < 127; i++) {
    Wire.beginTransmission(i);
    if(Wire.endTransmission() == 0) {
      Serial.print("Found I2C device at 0x");
      Serial.println(i, HEX);
    }
  }
}

void loop() {}
```

Alamat yang umum:
- 0x27 (paling umum)
- 0x3F (alternatif)

---

## Troubleshooting

### ESP32 #1
- **Lampu tidak nyala**: Cek relay, pastikan common dan NO terhubung ke lampu
- **Gas sensor selalu 0**: Panaskan sensor 2-3 menit dulu (preheat)
- **Servo tidak gerak**: Cek power supply, servo butuh arus besar

### ESP32 #2
- **Warna tidak terdeteksi**: Kalibrasi TCS3200, pastikan jarak 1-2cm dari objek
- **Metal detector tidak sensitif**: Adjust jarak sensor, max 5mm
- **Servo patah-patah**: Gunakan external power 5V 2A

### ESP32 #3
- **LCD blank**: Cek alamat I2C (0x27 atau 0x3F), putar potensiometer kontras
- **RFID tidak baca**: Cek koneksi SPI, pastikan kartu 13.56MHz
- **Keypad tidak respon**: Cek koneksi matrix, pastikan tidak ada short
- **Solenoid tidak kuat**: Gunakan 12V power supply, bukan 5V

---

## GPIO Notes

**GPIO yang AMAN digunakan:**
- GPIO 2, 4, 5, 12, 13, 14, 15, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35

**GPIO yang HARUS DIHINDARI:**
- GPIO 0: Boot mode (jangan gunakan)
- GPIO 1, 3: UART TX/RX (untuk Serial Monitor)
- GPIO 6-11: Flash memory (JANGAN GUNAKAN!)

**Input Only (tidak bisa output):**
- GPIO 34, 35, 36, 39: Hanya untuk input analog

---

## Library Requirements

Install library berikut di Arduino IDE:

```
- PubSubClient (MQTT)
- MFRC522 (RFID)
- LiquidCrystal_I2C (LCD)
- Keypad (Matrix Keypad)
- ESP32Servo (Servo control)
```

Cara install: Arduino IDE → Tools → Manage Libraries → Search & Install

---

Semua pin sudah dioptimalkan untuk menghindari konflik. Upload kode dan test satu per satu!


---

## ESP32 #4 - WiFi Controller & LCD Display

### Fungsi Utama
- WiFi Controller untuk semua ESP32
- LCD Display untuk monitoring semua aktivitas
- Config Mode (Access Point)
- Forward WiFi credentials via Serial TX

### Pin Configuration

#### Serial Communication (TX to Slaves)
| Pin | Function | Connection |
|-----|----------|------------|
| GPIO 17 | TX to ESP32 #1 | → ESP32 #1 RX (GPIO 16) |
| GPIO 16 | TX to ESP32 #2 | → ESP32 #2 RX (GPIO 16) |
| GPIO 1 (TX) | TX to ESP32 #3 | → ESP32 #3 RX (GPIO 16) |

#### LCD Display (I2C)
| Pin | Function | Connection |
|-----|----------|------------|
| GPIO 21 (SDA) | I2C Data | LCD SDA |
| GPIO 22 (SCL) | I2C Clock | LCD SCL |

#### Buzzer
| Pin | Function | Connection |
|-----|----------|------------|
| GPIO 14 | Buzzer Control | Buzzer (+) |

### Wiring Diagram

```
ESP32 #4
├── GPIO 17 (TX1) ──────→ ESP32 #1 GPIO 16 (RX)
├── GPIO 16 (TX2) ──────→ ESP32 #2 GPIO 16 (RX)
├── GPIO 1 (TX) ────────→ ESP32 #3 GPIO 16 (RX)
├── GPIO 21 (SDA) ──────→ LCD SDA
├── GPIO 22 (SCL) ──────→ LCD SCL
├── GPIO 14 ────────────→ Buzzer (+)
└── GND ────────────────→ Common Ground (All ESP32s + LCD + Buzzer)
```

### Power Supply
- **Voltage**: 5V via USB or VIN pin
- **Current**: ~500mA (ESP32 + LCD + Buzzer)
- **Recommendation**: Use dedicated 5V 2A power supply

### LCD Display (16x2 I2C)
- **Address**: 0x27 (default) or 0x3F
- **Backlight**: Always ON
- **Display Mode**: Rotating (3 seconds interval)
  - ESP32 #1 Status (Lamp, Gas, Feeder)
  - ESP32 #2 Status (Trash, Clothesline)
  - ESP32 #3 Status (Door Lock)

### Serial Communication Protocol
- **Baud Rate**: 9600
- **Format**: `WIFI:SSID:PASSWORD\n`
- **Example**: `WIFI:MyHomeWiFi:password123\n`

### Config Mode (Access Point)
- **SSID**: `ESP32-Setup`
- **Password**: `12345678`
- **IP Address**: `192.168.4.1`
- **Web Server**: Port 80
- **DNS**: Captive portal enabled

### Notes
- ESP32 #4 is the WiFi controller for all devices
- All WiFi configuration goes through ESP32 #4
- LCD shows real-time status from all ESP32s
- Buzzer beeps during intro animation
- Auto-restart after WiFi config
