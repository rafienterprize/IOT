# 📟 ESP32 Code - Final Setup

## 🎯 File yang Dipakai (3 ESP32)

Kamu butuh **3 ESP32** untuk semua fitur:

### ESP32 #1 - Smart Lamp, Gas Detector, Fish Feeder
**File:** `ESP32_1_Lamp_Gas_Feeder.ino`

**Device:**
- 💡 Smart Lamp (ON/OFF + Timer)
- 🌫️ Gas Detector (MQ-2/MQ-135)
- 🐟 Fish Feeder (Servo)

**Pins:**
- GPIO 2 → Relay (Lamp)
- GPIO 34 → Gas Sensor (Analog)
- GPIO 18 → Servo (Feeder)

---

### ESP32 #2 - Smart Trash Sorting, Smart Clothesline
**File:** `ESP32_2_Trash_Clothesline_Updated.ino`

**Device:**
- 🗑️ Smart Trash Sorting (3 bins dengan auto-sorting)
- 👕 Smart Clothesline (Auto rain detection)

**Pins Trash:**
- GPIO 5, 18 → Ultrasonik Organik
- GPIO 19, 21 → Ultrasonik Anorganik
- GPIO 22, 23 → Ultrasonik Metal
- GPIO 32-27 → Color Sensor TCS3200
- GPIO 34 → Metal Detector
- GPIO 13 → Servo Rotation
- GPIO 12 → Servo Gate

**Pins Clothesline:**
- GPIO 35 → Rain Sensor
- GPIO 14, 15 → Motor Driver

---

### ESP32 #3 - Smart Door Lock
**File:** `ESP32_3_Smart_Door.ino`

**Device:**
- 🚪 Smart Door Lock (RFID + PIN + LCD)

**Pins:**
- GPIO 21, 18, 23, 19, 22 → RFID RC522 (SPI)
- GPIO 21, 22 → LCD I2C 16x2
- GPIO 32, 33, 25, 26, 27, 14, 12, 13 → Keypad 4x4
- GPIO 13 → Door Lock (Relay/Solenoid)
- GPIO 14 → Buzzer

---

## 🔧 Cara Upload

### 1. Install Library yang Dibutuhkan

Buka Arduino IDE → Tools → Manage Libraries, install:

**Untuk Semua ESP32:**
- `PubSubClient` by Nick O'Leary
- `ESP32Servo` by Kevin Harrington

**Untuk ESP32 #3 (Smart Door):**
- `MFRC522` by GithubCommunity
- `LiquidCrystal I2C` by Frank de Brabander
- `Keypad` by Mark Stanley

### 2. Edit WiFi Credentials

Di setiap file .ino, edit:
```cpp
const char* ssid = "NAMA_WIFI_KAMU";
const char* password = "PASSWORD_WIFI_KAMU";
```

### 3. Upload ke ESP32

**ESP32 #1:**
1. Buka `ESP32_1_Lamp_Gas_Feeder.ino`
2. Pilih Board: ESP32 Dev Module
3. Pilih Port COM
4. Upload

**ESP32 #2:**
1. Buka `ESP32_2_Trash_Clothesline_Updated.ino`
2. Upload ke ESP32 kedua

**ESP32 #3:**
1. Buka `ESP32_3_Smart_Door.ino`
2. Upload ke ESP32 ketiga

### 4. Test dengan Serial Monitor

Buka Serial Monitor (115200 baud) untuk setiap ESP32:
```
Connecting to WiFi...
WiFi connected
IP address: 192.168.1.XXX
Attempting MQTT connection...connected
```

---

## 📊 Device Mapping

| ESP32 | Devices | MQTT Topics Prefix |
|-------|---------|-------------------|
| #1 | Lamp, Gas, Feeder | `iot/lamp/`, `iot/gas/`, `iot/feeder/` |
| #2 | Trash, Clothesline | `iot/trash/`, `iot/clothesline/` |
| #3 | Door Lock | `iot/door/` |

---

## 🎯 Alternatif: Pakai 2 ESP32 Saja

Kalau cuma punya 2 ESP32, bisa gabungkan:

### Option A: Gabung Door ke ESP32 #1
ESP32 #1: Lamp + Gas + Feeder + Door
ESP32 #2: Trash + Clothesline

### Option B: Gabung Feeder ke ESP32 #2
ESP32 #1: Lamp + Gas + Door
ESP32 #2: Trash + Clothesline + Feeder

**Cara gabung:**
1. Copy semua pin definitions
2. Copy semua callback handlers
3. Copy semua loop functions
4. Pastikan tidak ada pin conflict

---

## 🐛 Troubleshooting

### ESP32 tidak connect WiFi
```
Cek:
- SSID dan password benar
- WiFi 2.4GHz (bukan 5GHz)
- Jarak ke router
```

### MQTT tidak connect
```
Cek:
- WiFi sudah connect
- Broker address benar
- Port benar (1883 untuk ESP32)
- Serial Monitor untuk error
```

### Sensor tidak terbaca
```
Cek:
- Wiring benar
- Power supply cukup
- Pin number di code
```

### Upload error
```
Solusi:
- Tekan tombol BOOT saat upload
- Ganti kabel USB
- Install driver CH340/CP2102
```

---

## 📝 Checklist Upload

Sebelum upload, pastikan:

- [ ] WiFi credentials sudah diubah
- [ ] MQTT broker address benar
- [ ] Library sudah terinstall
- [ ] Board ESP32 Dev Module dipilih
- [ ] Port COM benar
- [ ] Serial Monitor 115200 baud

---

## 🔗 Wiring Guides

Detail wiring untuk setiap ESP32:

- **ESP32 #1:** Lihat `README_ESP32.md`
- **ESP32 #2:** Lihat `SMART_TRASH_WIRING.md`
- **ESP32 #3:** Lihat `SMART_DOOR_WIRING.md`

---

## 💡 Tips

1. **Upload satu-satu** - Test ESP32 #1 dulu, baru #2, baru #3
2. **Label ESP32** - Kasih label fisik biar tidak bingung
3. **Backup code** - Simpan WiFi credentials di tempat aman
4. **Serial Monitor** - Selalu cek untuk debugging
5. **Power supply** - Gunakan power terpisah untuk motor/servo

---

## 🚀 Quick Start

```bash
1. Edit WiFi di 3 file .ino
2. Upload ESP32_1_Lamp_Gas_Feeder.ino → ESP32 pertama
3. Upload ESP32_2_Trash_Clothesline_Updated.ino → ESP32 kedua
4. Upload ESP32_3_Smart_Door.ino → ESP32 ketiga
5. Buka Serial Monitor untuk cek koneksi
6. Jalankan website: npm run dev
7. Test semua device dari dashboard!
```

---

## 📞 Support

Kalau ada masalah:
1. Cek Serial Monitor untuk error messages
2. Cek wiring sesuai guide
3. Test satu device dulu sebelum gabungkan semua
4. Pastikan MQTT broker running
