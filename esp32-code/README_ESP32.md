# 🔌 Panduan Setup ESP32

## 📋 Hardware yang Dibutuhkan

### ESP32 #1 (Smart Lamp, Gas Detector, Fish Feeder)
- 1x ESP32 Dev Board
- 1x Relay Module (untuk lampu)
- 1x Sensor Gas MQ-2 atau MQ-135
- 1x Servo Motor SG90 (untuk fish feeder)
- LED (opsional untuk testing)
- Kabel jumper

### ESP32 #2 (Smart Trash, Smart Clothesline)
- 1x ESP32 Dev Board
- 1x Sensor Ultrasonik HC-SR04 (untuk trash bin)
- 1x Sensor Hujan/Rain Sensor
- 1x Motor Driver L298N atau relay 2 channel (untuk jemuran)
- 1x DC Motor atau motor stepper (untuk jemuran)
- Kabel jumper

## 🔧 Koneksi Pin

### ESP32 #1
```
Smart Lamp:
- GPIO 2 → Relay IN → Lampu

Gas Detector:
- GPIO 34 (ADC) → MQ-2/MQ-135 AOUT
- VCC → 3.3V
- GND → GND

Fish Feeder:
- GPIO 18 → Servo Signal
- Servo VCC → 5V
- Servo GND → GND
```

### ESP32 #2
```
Trash Bin (Ultrasonik):
- GPIO 5 → TRIG
- GPIO 18 → ECHO
- VCC → 5V
- GND → GND

Rain Sensor:
- GPIO 34 (ADC) → AO
- VCC → 3.3V
- GND → GND

Clothesline Motor:
- GPIO 25 → Motor Driver IN1
- GPIO 26 → Motor Driver IN2
- Motor Driver → DC Motor
```

## 📝 Instalasi Software

### 1. Install Arduino IDE
Download dari: https://www.arduino.cc/en/software

### 2. Install ESP32 Board
1. Buka Arduino IDE
2. File → Preferences
3. Tambahkan URL ini di "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Tools → Board → Boards Manager
5. Cari "ESP32" dan install

### 3. Install Library yang Dibutuhkan
Tools → Manage Libraries, install:
- `PubSubClient` by Nick O'Leary
- `ESP32Servo` by Kevin Harrington

## 🚀 Upload Code ke ESP32

### ESP32 #1
1. Buka file `ESP32_1_Lamp_Gas_Feeder.ino`
2. Edit WiFi credentials:
   ```cpp
   const char* ssid = "NAMA_WIFI_KAMU";
   const char* password = "PASSWORD_WIFI_KAMU";
   ```
3. Pilih board: Tools → Board → ESP32 Dev Module
4. Pilih port: Tools → Port → (pilih port ESP32)
5. Upload!

### ESP32 #2
1. Buka file `ESP32_2_Trash_Clothesline.ino`
2. Edit WiFi credentials (sama seperti ESP32 #1)
3. Upload ke ESP32 kedua

## 🧪 Testing

### Test MQTT Connection
1. Buka Serial Monitor (115200 baud)
2. Pastikan muncul "WiFi connected" dan "MQTT connected"
3. Cek IP address yang muncul

### Test dari Website
1. Jalankan website Next.js (`npm run dev`)
2. Buka http://localhost:3000
3. Coba kontrol setiap device
4. Monitor Serial Monitor untuk melihat pesan yang diterima

## 🌐 Menggunakan MQTT Broker Sendiri

### Install Mosquitto (Opsional)
Jika ingin pakai broker lokal:

**Windows:**
```bash
# Download dari: https://mosquitto.org/download/
# Install dan jalankan service
```

**Linux/Mac:**
```bash
sudo apt-get install mosquitto mosquitto-clients
sudo systemctl start mosquitto
```

Edit di ESP32 code:
```cpp
const char* mqtt_server = "192.168.1.XXX"; // IP komputer kamu
const int mqtt_port = 1883;
```

Edit di `.env.local`:
```env
NEXT_PUBLIC_MQTT_BROKER=192.168.1.XXX
NEXT_PUBLIC_MQTT_PORT=1883
```

## 🐛 Troubleshooting

### ESP32 tidak connect ke WiFi
- Pastikan SSID dan password benar
- Cek jarak ke router
- Pastikan WiFi 2.4GHz (ESP32 tidak support 5GHz)

### MQTT tidak connect
- Cek koneksi internet
- Coba broker lain: `test.mosquitto.org`
- Pastikan port tidak diblok firewall

### Sensor tidak terbaca
- Cek koneksi kabel
- Cek pin number di code
- Test sensor dengan code sederhana dulu

## 📊 MQTT Topics Reference

Lihat README.md utama untuk daftar lengkap topics.

## 💡 Tips
- Gunakan power supply terpisah untuk motor dan servo (jangan dari ESP32)
- Tambahkan resistor pull-up/pull-down jika sensor tidak stabil
- Monitor Serial untuk debugging
- Test satu device dulu sebelum gabungkan semua
