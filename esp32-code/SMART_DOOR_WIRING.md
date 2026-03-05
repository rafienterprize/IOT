# 🚪 Smart Door Lock System - Wiring Guide

## Komponen yang Dibutuhkan

### Hardware
- 1x ESP32 Development Board
- 1x RFID RC522 Module
- 1x LCD I2C 16x2 (Hijau/Biru dengan 4 pin)
- 1x Keypad 4x4 Matrix
- 1x Solenoid Door Lock atau Servo
- 1x Buzzer
- 1x Relay Module (untuk solenoid)
- Kartu RFID / Tag RFID
- Kabel jumper

## Pin Connections ESP32

### RFID RC522 Module

```
SDA (SS) → GPIO 21
SCK → GPIO 18
MOSI → GPIO 23
MISO → GPIO 19
IRQ → (tidak dipakai)
GND → GND
RST → GPIO 22
3.3V → 3.3V
```

### LCD I2C 16x2 (4 Pin)

```
GND → GND
VCC → 5V
SDA → GPIO 21 (I2C Data)
SCL → GPIO 22 (I2C Clock)
```

**Catatan:** LCD I2C biasanya address 0x27 atau 0x3F. Cek dengan I2C scanner jika tidak muncul.

### Keypad 4x4 Matrix

```
Row 1 → GPIO 32
Row 2 → GPIO 33
Row 3 → GPIO 25
Row 4 → GPIO 26

Col 1 → GPIO 27
Col 2 → GPIO 14
Col 3 → GPIO 12
Col 4 → GPIO 13
```

### Door Lock (Solenoid via Relay)

```
Relay IN → GPIO 13
Relay VCC → 5V
Relay GND → GND

Solenoid → Relay NO/NC
Solenoid Power → 12V External
```

### Buzzer

```
Positive → GPIO 14
Negative → GND
```

## Keypad Layout

```
1  2  3  A
4  5  6  B
7  8  9  C
*  0  #  D
```

- **0-9**: Input PIN
- **#**: Submit PIN
- *****: Clear PIN
- **A-D**: Reserved (bisa untuk fungsi tambahan)

## LCD Display Messages

### Normal State
```
Line 1: Ready to Scan
Line 2: (kosong)
```

### RFID Scan
```
Line 1: Access Granted
Line 2: Welcome!
```

### PIN Entry
```
Line 1: Enter PIN:
Line 2: ****
```

### Registration Mode
```
Line 1: Tap Card Now
Line 2: [Nama User]
```

## Cara Kerja Sistem

### 1. RFID Access
```
Tap kartu → Cek database → Granted/Denied → Unlock/Beep
```

### 2. PIN Access
```
Tekan angka → Tampil * di LCD → Tekan # → Cek PIN → Unlock/Beep
```

### 3. Card Registration
```
Dashboard: Add Card → Input nama → ESP32 mode register → Tap kartu → Saved
```

### 4. PIN Registration
```
Dashboard: Add PIN → Input 4 digit → Saved to ESP32
```

## MQTT Topics

### Subscribe (ESP32 menerima)
```
iot/door/control              → LOCK/UNLOCK
iot/door/emergency            → Emergency unlock
iot/door/rfid/register        → {"mode":"START","name":"John"}
iot/door/rfid/delete          → Card UID to delete
iot/door/pin/register         → {"pin":"1234"}
iot/door/pin/delete           → PIN to delete
```

### Publish (ESP32 mengirim)
```
iot/door/status               → LOCKED/UNLOCKED
iot/door/lcd                  → LCD message update
iot/door/rfid/scan            → {"uid":"ABC123","status":"ACCESS_GRANTED"}
iot/door/rfid/registered      → {"uid":"ABC123","name":"John","addedAt":"..."}
iot/door/pin/entry            → {"pin":"1234","status":"ACCESS_GRANTED"}
iot/door/access/log           → {"time":"...","method":"RFID","status":"GRANTED","identifier":"..."}
iot/esp32_3/heartbeat         → ONLINE
```

## Library yang Dibutuhkan

Install via Arduino Library Manager:

1. **MFRC522** by GithubCommunity
2. **LiquidCrystal I2C** by Frank de Brabander
3. **Keypad** by Mark Stanley
4. **PubSubClient** by Nick O'Leary

## Kalibrasi & Testing

### Test LCD I2C
```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.print("Hello World!");
}
```

Jika tidak muncul, coba address 0x3F.

### Test RFID
```cpp
#include <SPI.h>
#include <MFRC522.h>

MFRC522 mfrc522(21, 22);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();
    mfrc522.PICC_HaltA();
  }
}
```

### Test Keypad
```cpp
#include <Keypad.h>

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

void setup() {
  Serial.begin(115200);
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    Serial.println(key);
  }
}
```

## Troubleshooting

### LCD Tidak Muncul
- Cek koneksi SDA/SCL
- Coba address 0x3F
- Adjust potentiometer di belakang LCD untuk contrast
- Test dengan I2C scanner

### RFID Tidak Terbaca
- Pastikan jarak kartu < 3cm
- Cek koneksi SPI
- Pastikan power 3.3V (bukan 5V!)
- Test dengan code sederhana

### Keypad Tidak Responsif
- Cek semua koneksi row/col
- Test satu tombol dulu
- Pastikan tidak ada short circuit

### Solenoid Tidak Kuat
- Gunakan power supply 12V terpisah
- Pastikan relay rated untuk solenoid
- Cek koneksi relay

## Security Tips

⚠️ **PENTING:**
1. Simpan registered cards di EEPROM untuk persistent storage
2. Encrypt PIN sebelum simpan
3. Limit failed attempts (3x salah = lock 1 menit)
4. Log semua access attempts
5. Backup power untuk lock (battery)
6. Emergency mechanical key backup

## Upgrade Ideas

1. **Fingerprint Sensor** - Tambah biometric
2. **Camera** - ESP32-CAM untuk foto setiap akses
3. **Telegram Bot** - Notifikasi real-time
4. **NFC** - Support smartphone NFC
5. **Face Recognition** - AI-based access
6. **Time-based Access** - Restrict access by time
7. **Multi-factor** - RFID + PIN required

## Power Supply

- ESP32: 5V 1A via USB
- Solenoid: 12V 2A external
- Total system: 12V 3A recommended

## Wiring Diagram

```
ESP32
├── RFID RC522 (SPI)
├── LCD I2C (I2C)
├── Keypad 4x4 (GPIO)
├── Relay → Solenoid Lock
└── Buzzer

External 12V PSU → Solenoid
```
