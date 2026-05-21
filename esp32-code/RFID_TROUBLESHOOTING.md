# ESP32 #3 RFID Troubleshooting Guide

## Masalah: Tidak Bisa Scan Kartu di Website

### Langkah Debug:

## 1. **Cek MQTT Connection**
Buka Serial Monitor ESP32 #3, pastikan ada:
```
✓ MQTT Connected & Heartbeat sent!
```

Jika tidak ada, cek:
- WiFi connection
- MQTT broker (broker.hivemq.com)

## 2. **Test Scan Mode Activation**
Di website SmartGate:
1. Klik "Tambah Kartu"
2. Klik "📡 Mulai Scan Kartu"

Di Serial Monitor harus muncul:
```
Message arrived [iot/gate/rfid/scanmode]: START
========================================
Gate RFID scan mode activated!
Ready to scan cards for gate access...
========================================
```

## 3. **Test RFID Card Detection**
Setelah scan mode aktif, tempelkan kartu RFID ke reader.

Di Serial Monitor harus muncul:
```
========================================
Card detected: A1B2C3D4
Card scanned in scan mode: A1B2C3D4
Publishing to iot/gate/rfid/scan...
RFID scan data published: {"uid":"A1B2C3D4","status":"SCANNED"}
========================================
```

## 4. **Cek Website Response**
Setelah kartu di-scan, di website harus muncul:
- UID kartu di kotak "Scanned UID"
- Tombol "Simpan" menjadi aktif

## 5. **Test Card Registration**
1. Isi nama kartu
2. Klik "Simpan"

Di Serial Monitor harus muncul:
```
Message arrived [iot/gate/rfid/register]: {"uid":"A1B2C3D4","name":"John Doe"}
Card registered: A1B2C3D4 - John Doe
```

---

## Kemungkinan Masalah:

### **RFID Hardware**
**Gejala**: Tidak ada "Card detected" di Serial Monitor
**Solusi**:
```
1. Cek wiring RFID RC522:
   - VCC → 3.3V (BUKAN 5V!)
   - GND → GND
   - RST → GPIO 22
   - SS/SDA → GPIO 21
   - MOSI → GPIO 23
   - MISO → GPIO 19
   - SCK → GPIO 18

2. Test dengan kode sederhana:
```cpp
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 22
#define SS_PIN 21

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID Ready");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    Serial.println("Card UID: " + uid);
    mfrc522.PICC_HaltA();
  }
  delay(100);
}
```

### **MQTT Topics Salah**
**Gejala**: Scan mode tidak aktif
**Cek Topics**:
- Website publish: `iot/gate/rfid/scanmode` → START
- ESP32 subscribe: `iot/gate/rfid/scanmode`
- ESP32 publish: `iot/gate/rfid/scan` → {"uid":"...", "status":"SCANNED"}
- Website subscribe: `iot/gate/rfid/scan`

### **Scan Mode Tidak Aktif**
**Gejala**: Kartu terdeteksi tapi tidak masuk scan mode
**Debug**:
```cpp
// Tambahkan di loop() ESP32
Serial.println("Scan mode: " + String(isScanMode ? "ACTIVE" : "INACTIVE"));
```

### **Website Tidak Terima Data**
**Gejala**: ESP32 publish tapi website tidak update
**Cek**:
1. Browser console (F12) untuk error
2. MQTT connection di website
3. Topic subscription yang benar

---

## Manual Test Commands:

### Test RFID Reader
Di Serial Monitor ESP32 #3, ketik:
```
AT+RFID_TEST
```

### Test Scan Mode
Di Serial Monitor ESP32 #3, ketik:
```
AT+SCAN_MODE_ON
AT+SCAN_MODE_OFF
```

### Test Card Registration
Di Serial Monitor ESP32 #3, ketik:
```
AT+REGISTER_CARD:A1B2C3D4:TestCard
```

---

## Wiring Diagram RFID RC522:

```
ESP32 #3          RFID RC522
GPIO 23    ────→  MOSI
GPIO 19    ────→  MISO  
GPIO 18    ────→  SCK
GPIO 21    ────→  SDA/SS
GPIO 22    ────→  RST
3.3V       ────→  VCC (PENTING: 3.3V, bukan 5V!)
GND        ────→  GND
```

## Kartu RFID yang Didukung:
- **Frequency**: 13.56MHz
- **Type**: MIFARE Classic 1K
- **Format**: ISO14443A

**TIDAK DIDUKUNG**:
- 125kHz cards (EM4100, T5577)
- UHF RFID cards
- NFC cards yang tidak kompatibel

---

## Quick Fix:

### Reset RFID System
```cpp
// Tambahkan di setup()
mfrc522.PCD_Reset();
delay(100);
mfrc522.PCD_Init();
```

### Force Scan Mode
```cpp
// Tambahkan di loop() untuk test
static bool testMode = false;
if (!testMode) {
  isScanMode = true;
  testMode = true;
  Serial.println("Force scan mode ON");
}
```

Coba langkah-langkah ini dan kasih tau output Serial Monitor-nya!