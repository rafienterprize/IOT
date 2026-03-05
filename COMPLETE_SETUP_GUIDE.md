# 🚀 Panduan Lengkap Setup ESP32 dengan Website IoT

## 📋 Yang Kamu Butuhkan

### Hardware
- 3x ESP32 Development Board
- Kabel USB Type-C/Micro-USB (untuk upload code)
- Komponen sensor sesuai project (lihat wiring guide)
- Laptop/PC Windows

### Software
- Arduino IDE (download dari arduino.cc)
- Node.js 18+ (download dari nodejs.org)
- Browser modern (Chrome/Firefox/Edge)
- Kabel USB driver (biasanya auto-install)

---

## 🎯 STEP 1: Setup Website (15 menit)

### 1.1 Install Dependencies

Buka terminal/PowerShell di folder project:

```bash
npm install
```

Tunggu sampai selesai (sekitar 2-5 menit).

### 1.2 Jalankan Website

```bash
npm run dev
```

Tunggu sampai muncul:
```
✓ Ready in 4.6s
- Local:   http://localhost:3000
- Network: http://192.168.1.XXX:3000
```

### 1.3 Test Website

1. Buka browser
2. Akses: http://localhost:3000
3. Kamu akan lihat dashboard dengan sidebar
4. Status MQTT akan "Disconnected" (normal, karena ESP32 belum connect)

✅ **Website sudah jalan!**

**Catatan MQTT:**
Website sudah dikonfigurasi menggunakan **HiveMQ Public Broker** (`broker.hivemq.com`). Ini artinya:
- ✅ Tidak perlu install MQTT broker
- ✅ Langsung bisa connect
- ✅ Gratis selamanya
- ✅ ESP32 dan website otomatis connect via internet

Status MQTT akan berubah jadi "Connected" (hijau) setelah ESP32 online.

---

## 🎯 STEP 2: Setup MQTT (OPSIONAL - Bisa Diskip!)

### Pilihan A: Pakai HiveMQ Public (Recommended untuk Pemula)

**Tidak perlu setup apa-apa!** Website dan ESP32 code sudah dikonfigurasi pakai HiveMQ.

**Kelebihan:**
- ✅ Zero setup
- ✅ Langsung jalan
- ✅ Bisa akses dari mana aja (internet)

**Kekurangan:**
- ⚠️ Agak lambat (server di luar negeri)
- ⚠️ Butuh internet

**Skip ke STEP 3 jika pakai HiveMQ!**

---

### Pilihan B: Install Mosquitto Lokal (Advanced - Lebih Cepat)

Jika mau performa lebih cepat dan tidak bergantung internet:

#### B.1 Download & Install Mosquitto

1. Buka: https://mosquitto.org/download/
2. Download: `mosquitto-2.0.18-install-windows-x64.exe`
3. Install dengan default settings
4. Centang "Install as service"

#### B.2 Konfigurasi WebSocket

Buka Notepad **as Administrator**, edit file:
```
C:\Program Files\mosquitto\mosquitto.conf
```

Tambahkan di akhir file:
```conf
listener 1883
listener 9001
protocol websockets
allow_anonymous true
```

Save file.

#### B.3 Restart Service

Buka PowerShell **as Administrator**:
```powershell
net stop mosquitto
net start mosquitto
```

#### B.4 Allow Firewall

```powershell
netsh advfirewall firewall add rule name="Mosquitto MQTT" dir=in action=allow protocol=TCP localport=1883
netsh advfirewall firewall add rule name="Mosquitto WebSocket" dir=in action=allow protocol=TCP localport=9001
```

#### B.5 Cek IP Komputer

```powershell
ipconfig
```

Catat IP address (contoh: 192.168.1.100)

#### B.6 Update Website

Edit file `.env.local`:
```env
NEXT_PUBLIC_MQTT_BROKER=192.168.1.100
NEXT_PUBLIC_MQTT_PORT=9001
```

Restart website:
```bash
# Ctrl+C untuk stop
npm run dev
```

#### B.7 Update ESP32 Code

Edit di **SEMUA** file .ino (ESP32_1, ESP32_2, ESP32_3):
```cpp
// Ganti ini:
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Jadi ini:
const char* mqtt_server = "192.168.1.100";  // IP komputer kamu
const int mqtt_port = 1883;
```

Upload ulang ke semua ESP32.

✅ **Mosquitto lokal sudah jalan!**

**Perbandingan:**

| Feature | HiveMQ Public | Mosquitto Lokal |
|---------|---------------|-----------------|
| Setup Time | 0 menit | 15 menit |
| Speed | ⭐⭐⭐ (200-500ms) | ⭐⭐⭐⭐⭐ (<10ms) |
| Internet | Wajib | Tidak perlu |
| Recommended | Pemula/Testing | Production/Daily |

**Rekomendasi Aku:**
- **Minggu 1-2:** Pakai HiveMQ (fokus ke hardware)
- **Minggu 3+:** Upgrade ke Mosquitto (lebih cepat)

---

## 🎯 STEP 3: Setup Arduino IDE (10 menit)

### 2.1 Install Arduino IDE

1. Download dari: https://www.arduino.cc/en/software
2. Install dengan default settings
3. Buka Arduino IDE

### 2.2 Install ESP32 Board Support

1. Buka Arduino IDE
2. Klik **File → Preferences**
3. Di "Additional Board Manager URLs", paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Klik **OK**
5. Klik **Tools → Board → Boards Manager**
6. Cari "ESP32"
7. Install **"ESP32 by Espressif Systems"**
8. Tunggu sampai selesai (5-10 menit)

### 2.3 Install Library yang Dibutuhkan

Klik **Tools → Manage Libraries**, cari dan install:

**Untuk Semua ESP32:**
- `PubSubClient` by Nick O'Leary
- `ESP32Servo` by Kevin Harrington

**Untuk ESP32 #3 (Smart Door):**
- `MFRC522` by GithubCommunity
- `LiquidCrystal I2C` by Frank de Brabander
- `Keypad` by Mark Stanley

✅ **Arduino IDE siap dipakai!**

---

## 🎯 STEP 4: Upload Code ke ESP32 (Per ESP32: 5 menit)

### 3.1 Persiapan

1. Colok ESP32 ke laptop via USB
2. Tunggu driver auto-install
3. Cek di Device Manager → Ports (COM & LPT)
4. Catat nomor COM port (contoh: COM3, COM5)

### 3.2 Upload ESP32 #1 (Lamp, Gas, Feeder)

#### A. Buka File
1. Buka Arduino IDE
2. File → Open
3. Pilih: `esp32-code/ESP32_1_Lamp_Gas_Feeder.ino`

#### B. Edit WiFi (Sementara)
Cari baris ini dan edit:
```cpp
const char* ssid = "NAMA_WIFI_KAMU";
const char* password = "PASSWORD_WIFI_KAMU";
```

Ganti dengan WiFi kamu atau pakai hotspot HP dulu.

#### C. Konfigurasi Board
1. **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
2. **Tools → Port → COM3** (sesuai port kamu)
3. **Tools → Upload Speed → 115200**

#### D. Upload
1. Klik tombol **Upload** (ikon panah kanan)
2. Tunggu proses compile (1-2 menit)
3. Jika muncul "Connecting...", tekan tombol **BOOT** di ESP32
4. Tunggu sampai muncul "Hard resetting via RTS pin..."

#### E. Test
1. Klik **Tools → Serial Monitor**
2. Set baud rate: **115200**
3. Kamu akan lihat:
   ```
   Connecting to WiFi...
   WiFi connected
   IP address: 192.168.1.XXX
   Attempting MQTT connection...connected
   ```

✅ **ESP32 #1 berhasil connect!**

### 3.3 Upload ESP32 #2 (Trash, Clothesline)

Ulangi langkah yang sama dengan file:
`esp32-code/ESP32_2_Trash_Clothesline_Updated.ino`

**PENTING:** Cabut ESP32 #1 dulu, baru colok ESP32 #2 (biar tidak bingung port nya)

### 3.4 Upload ESP32 #3 (Smart Door)

Ulangi langkah yang sama dengan file:
`esp32-code/ESP32_3_Smart_Door.ino`

✅ **Semua ESP32 sudah di-upload!**

---

## 🎯 STEP 5: Hubungkan ESP32 dengan Website (5 menit)

### 4.1 Cek Status di Website

1. Buka website: http://localhost:3000
2. Klik menu **"Device Status"** di sidebar
3. Kamu akan lihat status 3 ESP32:
   - ESP32 #1: 🟢 Online (jika sudah connect)
   - ESP32 #2: 🟢 Online
   - ESP32 #3: 🟢 Online

Jika masih offline, tunggu 10-30 detik (heartbeat interval).

### 4.2 Test Kontrol Device

#### Test Smart Lamp:
1. Klik menu **"Smart Lamp"**
2. Klik tombol **"NYALA"**
3. Cek Serial Monitor ESP32 #1, akan muncul:
   ```
   Message arrived [iot/lamp/control]: ON
   ```
4. LED/Relay akan nyala (jika sudah diwiring)

#### Test Smart Door:
1. Klik menu **"Smart Door"**
2. Klik tombol **"Unlock Door"**
3. Cek Serial Monitor ESP32 #3
4. LCD akan tampil "Door UNLOCKED"

#### Test Lainnya:
- Gas Detector: Lihat grafik real-time
- Fish Feeder: Klik "Beri Makan Sekarang"
- Smart Trash: Lihat level 3 bin
- Clothesline: Klik "Buka Jemuran"

✅ **ESP32 dan Website sudah terhubung!**

---

## 🎯 STEP 6: Ganti WiFi dari Website (Opsional)

Sekarang kamu bisa ganti WiFi tanpa upload code ulang!

### 5.1 Cara Ganti WiFi

1. Buka menu **"Device Status"**
2. Klik **"WiFi Pairing Mode"**
3. Masukkan:
   - **WiFi SSID**: Nama WiFi baru
   - **WiFi Password**: Password WiFi baru
4. Klik tombol ESP32 yang mau diganti WiFi:
   - **ESP32 #1** (Lamp, Gas, Feeder)
   - **ESP32 #2** (Trash, Clothesline)
   - **ESP32 #3** (Smart Door)
5. ESP32 akan restart dan connect ke WiFi baru
6. Tunggu 30 detik, status akan online lagi

✅ **WiFi berhasil diganti!**

---

## 🎯 STEP 7: Wiring Hardware (Per Device)

Sekarang hubungkan sensor dan actuator ke ESP32.

### ESP32 #1 - Wiring

**Smart Lamp:**
- GPIO 2 → Relay IN
- Relay VCC → 5V
- Relay GND → GND
- Relay NO/COM → Lampu

**Gas Detector:**
- MQ-2 AOUT → GPIO 34
- MQ-2 VCC → 3.3V
- MQ-2 GND → GND

**Fish Feeder:**
- Servo Signal → GPIO 18
- Servo VCC → 5V (external)
- Servo GND → GND

Detail lengkap: `esp32-code/README_ESP32.md`

### ESP32 #2 - Wiring

Lihat: `esp32-code/SMART_TRASH_WIRING.md`

### ESP32 #3 - Wiring

Lihat: `esp32-code/SMART_DOOR_WIRING.md`

---

## 🐛 Troubleshooting

### Website tidak bisa dibuka

**Solusi:**
```bash
# Stop server (Ctrl+C)
# Hapus cache
rm -rf .next node_modules
npm install
npm run dev
```

### ESP32 tidak connect WiFi

**Cek:**
- [ ] SSID dan password benar
- [ ] WiFi 2.4GHz (bukan 5GHz)
- [ ] Jarak ke router tidak terlalu jauh
- [ ] Serial Monitor untuk lihat error

**Solusi:**
1. Buka Serial Monitor (115200 baud)
2. Tekan tombol RESET di ESP32
3. Lihat pesan error
4. Edit WiFi credentials dan upload ulang

### MQTT tidak connect

**Cek Serial Monitor:**
```
Attempting MQTT connection...failed, rc=-2
```

**Error Codes:**
- `rc=-2`: Network error, cek WiFi
- `rc=-4`: Timeout, cek broker address
- `rc=5`: Auth failed (tidak relevan untuk HiveMQ public)

**Solusi 1: Ganti Broker**

Edit di ESP32 code:
```cpp
const char* mqtt_server = "test.mosquitto.org";
```

Upload ulang.

**Solusi 2: Cek Firewall**

Pastikan port 1883 tidak diblok firewall.

**Solusi 3: Test dengan MQTT Explorer**

1. Download MQTT Explorer: http://mqtt-explorer.com/
2. Install dan buka
3. Add connection:
   - Host: `broker.hivemq.com`
   - Port: `1883`
4. Klik Connect
5. Jika berhasil connect, berarti broker OK
6. Jika gagal, coba broker lain atau install Mosquitto lokal

### Website MQTT Status "Disconnected"

**Solusi:**
1. Cek Serial Monitor, pastikan muncul "connected"
2. Tunggu 30 detik (heartbeat interval)
3. Refresh browser
4. Cek topic MQTT dengan MQTT Explorer

### Upload Error

**Error: "Failed to connect"**

**Solusi:**
1. Tekan dan tahan tombol **BOOT** di ESP32
2. Klik **Upload** di Arduino IDE
3. Lepas tombol BOOT setelah "Connecting..."

**Error: "Port not found"**

**Solusi:**
1. Install driver CH340 atau CP2102
2. Ganti kabel USB
3. Coba port USB lain

### Sensor tidak terbaca

**Solusi:**
1. Cek wiring dengan multimeter
2. Test sensor dengan code sederhana
3. Cek voltage (3.3V vs 5V)
4. Pastikan pin number benar di code

---

## 📊 Monitoring & Debugging

### 1. Serial Monitor (Arduino IDE)

Untuk debug ESP32:
```
Tools → Serial Monitor (115200 baud)
```

Kamu akan lihat:
- WiFi connection status
- MQTT connection status
- Sensor readings
- Error messages

### 2. MQTT Explorer (Highly Recommended!)

**Download:** http://mqtt-explorer.com/

**Setup untuk HiveMQ:**
1. Install MQTT Explorer
2. Add connection:
   - Name: `HiveMQ Public`
   - Host: `broker.hivemq.com`
   - Port: `1883`
3. Connect

**Setup untuk Mosquitto Lokal:**
1. Add connection:
   - Name: `Local Mosquitto`
   - Host: `localhost` (atau IP komputer)
   - Port: `1883`
3. Connect

**Kegunaan:**
- Monitor semua MQTT traffic
- Test publish/subscribe manual
- Debug connection issues
- Lihat message content

### 3. Browser Console

Untuk debug website:
1. Tekan F12 di browser
2. Tab Console
3. Lihat error MQTT connection
4. Cek network requests

---

## 🎯 Checklist Setup Lengkap

### MQTT Broker
- [ ] Pilih broker: HiveMQ Public atau Mosquitto Lokal
- [ ] Jika Mosquitto: Service running
- [ ] Jika Mosquitto: Firewall configured
- [ ] MQTT Explorer bisa connect ke broker

### Website
- [ ] `npm install` berhasil
- [ ] `npm run dev` jalan
- [ ] Browser bisa buka http://localhost:3000
- [ ] Dashboard muncul dengan sidebar

### Arduino IDE
- [ ] ESP32 board support terinstall
- [ ] Library PubSubClient terinstall
- [ ] Library ESP32Servo terinstall
- [ ] Library MFRC522 terinstall (untuk ESP32 #3)
- [ ] Library LiquidCrystal I2C terinstall (untuk ESP32 #3)
- [ ] Library Keypad terinstall (untuk ESP32 #3)

### ESP32 #1
- [ ] Code di-upload
- [ ] Serial Monitor tampil "WiFi connected"
- [ ] Serial Monitor tampil "MQTT connected"
- [ ] Status di website: 🟢 Online
- [ ] Test kontrol lamp berhasil

### ESP32 #2
- [ ] Code di-upload
- [ ] Serial Monitor tampil "WiFi connected"
- [ ] Serial Monitor tampil "MQTT connected"
- [ ] Status di website: 🟢 Online

### ESP32 #3
- [ ] Code di-upload
- [ ] Serial Monitor tampil "WiFi connected"
- [ ] Serial Monitor tampil "MQTT connected"
- [ ] Status di website: 🟢 Online
- [ ] LCD tampil "Ready to Scan"

### Wiring (Opsional untuk testing)
- [ ] Smart Lamp wiring selesai
- [ ] Gas Detector wiring selesai
- [ ] Fish Feeder wiring selesai
- [ ] Smart Trash wiring selesai
- [ ] Clothesline wiring selesai
- [ ] Smart Door wiring selesai

---

## 🚀 Quick Start (TL;DR)

Untuk yang mau cepat:

```bash
# 1. Setup Website
npm install
npm run dev
# Website pakai HiveMQ public broker (auto-configured)

# 2. Upload ESP32
# - Buka Arduino IDE
# - Edit WiFi di 3 file .ino
# - Upload ke 3 ESP32

# 3. Test
# - Buka http://localhost:3000
# - Cek Device Status
# - Test kontrol device

# Done! 🎉
```

---

## 📞 Bantuan

Jika masih ada masalah:

1. **Cek Serial Monitor** - 90% masalah keliatan di sini
2. **Cek MQTT Explorer** - Untuk debug MQTT traffic
3. **Cek Browser Console** - Untuk debug website
4. **Test satu-satu** - Jangan langsung semua device
5. **Restart semua** - ESP32, website, router

---

## 🎓 Tips untuk Demo/Presentasi

1. **Persiapan H-1:**
   - Test semua device
   - Charge power bank untuk ESP32
   - Backup WiFi credentials
   - Screenshot dashboard

2. **Saat Demo:**
   - Gunakan hotspot HP (lebih stabil)
   - Bawa laptop + ESP32 yang sudah diwiring
   - Siapkan MQTT Explorer untuk show traffic
   - Buka Serial Monitor untuk show real-time log

3. **Backup Plan:**
   - Video recording demo yang berhasil
   - Screenshot semua fitur
   - Diagram arsitektur sistem

---

## 🎉 Selamat!

Kamu berhasil setup sistem IoT monitoring lengkap dengan:
- ✅ Website dashboard real-time
- ✅ 3 ESP32 terhubung via MQTT
- ✅ 6 smart devices (Lamp, Gas, Feeder, Trash, Clothesline, Door)
- ✅ WiFi pairing dari website
- ✅ Device status monitoring
- ✅ Access control dengan RFID & PIN

**Next Steps:**
- Wiring semua sensor dan actuator
- Kustomisasi UI dashboard
- Tambah fitur notifikasi
- Deploy ke production

Good luck dengan project kamu! 🚀
