# 📡 Panduan Setup MQTT Broker

## Pilihan 1: Menggunakan Public Broker (Paling Mudah)

### HiveMQ Public Broker (Sudah dikonfigurasi)

Website dan ESP32 sudah dikonfigurasi menggunakan HiveMQ public broker.

**Konfigurasi saat ini:**
- Broker: `broker.hivemq.com`
- Port WebSocket: `8000` (untuk website)
- Port TCP: `1883` (untuk ESP32)
- Username/Password: Tidak perlu

**Kelebihan:**
✅ Gratis
✅ Tidak perlu install apa-apa
✅ Langsung bisa dipakai
✅ Akses dari mana saja (internet)

**Kekurangan:**
❌ Public (orang lain bisa lihat data jika tahu topic)
❌ Bergantung internet
❌ Bisa lambat jika server jauh

### Broker Public Lainnya

Jika HiveMQ lambat, bisa ganti ke:

**1. test.mosquitto.org**
```env
NEXT_PUBLIC_MQTT_BROKER=test.mosquitto.org
NEXT_PUBLIC_MQTT_PORT=8080
```

**2. broker.emqx.io**
```env
NEXT_PUBLIC_MQTT_BROKER=broker.emqx.io
NEXT_PUBLIC_MQTT_PORT=8083
```

Edit di file `.env.local` dan ESP32 code.

---

## Pilihan 2: Install Mosquitto Broker Lokal (Recommended)

### Windows

**1. Download Mosquitto**
- Kunjungi: https://mosquitto.org/download/
- Download installer Windows 64-bit
- Install dengan default settings

**2. Jalankan Mosquitto**
```powershell
# Buka PowerShell sebagai Administrator
cd "C:\Program Files\mosquitto"
.\mosquitto.exe -v
```

Atau jalankan sebagai service:
```powershell
net start mosquitto
```

**3. Konfigurasi untuk WebSocket**

Edit file `C:\Program Files\mosquitto\mosquitto.conf`:
```conf
# Default MQTT port
listener 1883

# WebSocket port untuk browser
listener 9001
protocol websockets

# Allow anonymous (untuk testing)
allow_anonymous true
```

Restart service:
```powershell
net stop mosquitto
net start mosquitto
```

**4. Test Koneksi**
```powershell
# Subscribe (terminal 1)
mosquitto_sub -h localhost -t test/topic

# Publish (terminal 2)
mosquitto_pub -h localhost -t test/topic -m "Hello MQTT"
```

### Linux/Mac

**1. Install Mosquitto**

Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install mosquitto mosquitto-clients
```

Mac (Homebrew):
```bash
brew install mosquitto
```

**2. Konfigurasi**

Edit `/etc/mosquitto/mosquitto.conf`:
```conf
listener 1883
listener 9001
protocol websockets
allow_anonymous true
```

**3. Start Service**
```bash
sudo systemctl start mosquitto
sudo systemctl enable mosquitto
```

**4. Test**
```bash
# Subscribe
mosquitto_sub -h localhost -t test/topic

# Publish
mosquitto_pub -h localhost -t test/topic -m "Hello"
```

---

## Konfigurasi Website & ESP32

### 1. Update Website (.env.local)

Untuk broker lokal:
```env
NEXT_PUBLIC_MQTT_BROKER=localhost
NEXT_PUBLIC_MQTT_PORT=9001
```

Untuk akses dari HP/device lain di jaringan yang sama:
```env
NEXT_PUBLIC_MQTT_BROKER=192.168.1.XXX  # IP komputer kamu
NEXT_PUBLIC_MQTT_PORT=9001
```

Cara cek IP komputer:
```bash
# Windows
ipconfig

# Linux/Mac
ifconfig
```

### 2. Update ESP32 Code

Edit di semua file ESP32 (.ino):
```cpp
// Ganti ini:
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Jadi ini (untuk broker lokal):
const char* mqtt_server = "192.168.1.XXX";  // IP komputer kamu
const int mqtt_port = 1883;
```

**PENTING:** ESP32 dan komputer harus di WiFi yang sama!

---

## Testing MQTT Connection

### 1. Test dengan MQTT Explorer (GUI Tool)

**Download:** http://mqtt-explorer.com/

**Setup:**
1. Install MQTT Explorer
2. Buka aplikasi
3. Klik "+" untuk add connection
4. Masukkan:
   - Name: `Local Broker`
   - Host: `localhost` atau `192.168.1.XXX`
   - Port: `1883`
5. Connect

Sekarang kamu bisa:
- Lihat semua topics
- Subscribe ke topic
- Publish message manual
- Monitor traffic real-time

### 2. Test dengan Command Line

**Subscribe ke semua topics:**
```bash
mosquitto_sub -h localhost -t '#' -v
```

**Publish test message:**
```bash
mosquitto_pub -h localhost -t iot/lamp/control -m "ON"
```

### 3. Test dari Website

1. Buka http://localhost:3000
2. Cek status koneksi di sidebar (hijau = connected)
3. Coba kontrol device
4. Monitor di MQTT Explorer atau terminal

---

## Troubleshooting

### Website tidak connect ke MQTT

**Cek 1: Firewall**
```powershell
# Windows - Allow port 9001
netsh advfirewall firewall add rule name="Mosquitto WebSocket" dir=in action=allow protocol=TCP localport=9001
```

**Cek 2: Browser Console**
- Buka browser (F12)
- Lihat tab Console
- Cari error MQTT connection

**Cek 3: Broker Running**
```bash
# Cek service status
# Windows
sc query mosquitto

# Linux
sudo systemctl status mosquitto
```

### ESP32 tidak connect ke MQTT

**Cek 1: WiFi Connection**
- Buka Serial Monitor (115200 baud)
- Pastikan muncul "WiFi connected"
- Cek IP address ESP32

**Cek 2: Broker Accessible**
- Ping broker dari komputer
- Pastikan ESP32 dan broker di network yang sama

**Cek 3: Port**
- ESP32 pakai port 1883 (TCP)
- Website pakai port 9001 (WebSocket)
- Jangan dibalik!

### Connection Timeout

**Solusi 1: Ganti Broker**
Coba broker public lain jika lokal bermasalah.

**Solusi 2: Restart**
```bash
# Restart Mosquitto
sudo systemctl restart mosquitto

# Restart ESP32
Tekan tombol RESET
```

---

## Security (Production)

Untuk production, jangan pakai `allow_anonymous true`!

### Setup Username & Password

**1. Create password file:**
```bash
# Windows
cd "C:\Program Files\mosquitto"
mosquitto_passwd -c passwords.txt admin

# Linux
sudo mosquitto_passwd -c /etc/mosquitto/passwords.txt admin
```

**2. Edit mosquitto.conf:**
```conf
listener 1883
listener 9001
protocol websockets

allow_anonymous false
password_file C:\Program Files\mosquitto\passwords.txt
```

**3. Update ESP32:**
```cpp
if (client.connect(clientId.c_str(), "admin", "password")) {
  // connected
}
```

**4. Update Website (.env.local):**
```env
NEXT_PUBLIC_MQTT_USERNAME=admin
NEXT_PUBLIC_MQTT_PASSWORD=password
```

Update `hooks/useMQTT.ts`:
```typescript
const client = mqtt.connect(url, {
  clientId: `iot_dashboard_${Math.random().toString(16).slice(3)}`,
  username: process.env.NEXT_PUBLIC_MQTT_USERNAME,
  password: process.env.NEXT_PUBLIC_MQTT_PASSWORD,
  clean: true,
});
```

---

## Cloud MQTT (Alternative)

Jika mau akses dari mana saja tanpa install:

### 1. CloudMQTT (Gratis tier available)
- Website: https://www.cloudmqtt.com/
- Free plan: 5 connections
- Setup mudah

### 2. HiveMQ Cloud
- Website: https://www.hivemq.com/mqtt-cloud-broker/
- Free tier available
- Dashboard bagus

### 3. AWS IoT Core
- Untuk production scale
- Bayar per message
- Integrasi AWS services

---

## Quick Start (Paling Cepat)

Kalau mau langsung jalan tanpa ribet:

**1. Pakai HiveMQ Public (sudah dikonfigurasi)**
- Tidak perlu install apa-apa
- Langsung `npm run dev`
- Upload ESP32 code
- Done!

**2. Nanti upgrade ke Mosquitto lokal**
- Lebih cepat
- Lebih aman
- Tidak perlu internet

---

## Monitoring & Debugging

### Lihat semua MQTT traffic:
```bash
mosquitto_sub -h localhost -t '#' -v
```

### Lihat topic spesifik:
```bash
# Smart Lamp
mosquitto_sub -h localhost -t 'iot/lamp/#' -v

# Smart Door
mosquitto_sub -h localhost -t 'iot/door/#' -v

# Semua heartbeat
mosquitto_sub -h localhost -t 'iot/+/heartbeat' -v
```

### Test publish manual:
```bash
# Nyalakan lampu
mosquitto_pub -h localhost -t iot/lamp/control -m "ON"

# Unlock door
mosquitto_pub -h localhost -t iot/door/control -m "UNLOCK"

# Beri makan ikan
mosquitto_pub -h localhost -t iot/feeder/control -m "FEED"
```

---

## Summary

**Untuk Development (Sekarang):**
✅ Pakai HiveMQ public broker (sudah setup)
✅ Tidak perlu install apa-apa
✅ Langsung bisa testing

**Untuk Production (Nanti):**
✅ Install Mosquitto lokal
✅ Setup username/password
✅ Lebih cepat dan aman

**Tools Recommended:**
- MQTT Explorer (GUI monitoring)
- mosquitto_sub/pub (CLI testing)
