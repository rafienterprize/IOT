# 🔌 Cara Menghubungkan ESP32 dengan Website via MQTT

## 🎯 Konsep Dasar

```
ESP32 ←→ MQTT Broker ←→ Website
```

- **ESP32**: Kirim/terima data via MQTT (port 1883)
- **MQTT Broker**: Perantara/server yang meneruskan pesan
- **Website**: Kirim/terima data via MQTT WebSocket (port 8000/9001)

---

## ✅ CARA TERMUDAH: Pakai HiveMQ Public (RECOMMENDED)

### Konfigurasi Saat Ini (Sudah Dikonfigurasi!)

**Website (`.env.local`):**
```env
NEXT_PUBLIC_MQTT_BROKER=broker.hivemq.com
NEXT_PUBLIC_MQTT_PORT=8000
```

**ESP32 Code (semua file .ino):**
```cpp
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
```

### Langkah Setup:

#### 1. Jalankan Website
```bash
npm run dev
```

Tunggu sampai muncul:
```
✓ Ready in 4.6s
- Local: http://localhost:3000
```

#### 2. Cek Status MQTT di Website

Buka browser → http://localhost:3000

Di sidebar kiri bawah, lihat status:
- 🔴 **Disconnected** = Website belum connect ke MQTT broker
- 🟢 **Connected** = Website sudah connect ke MQTT broker

**Jika Disconnected:**
1. Buka browser console (F12)
2. Lihat error message
3. Biasanya: "WebSocket connection failed"
4. **Solusi:** Tunggu 10-30 detik, atau refresh browser

#### 3. Upload ESP32 Code

Edit WiFi credentials di ESP32 code:
```cpp
const char* ssid = "NAMA_WIFI_KAMU";
const char* password = "PASSWORD_WIFI_KAMU";
```

Upload ke ESP32.

#### 4. Cek Serial Monitor ESP32

Buka Serial Monitor (115200 baud), kamu harus lihat:

```
Connecting to WiFi...
.....
WiFi connected
IP address: 192.168.1.XXX
Attempting MQTT connection...connected
```

**Jika stuck di "Attempting MQTT connection...":**
- Cek koneksi internet
- Coba ganti broker (lihat troubleshooting)

#### 5. Cek Device Status di Website

Buka menu "Device Status" di website.

Tunggu 10-30 detik, status ESP32 akan berubah:
- ESP32 #1: 🟢 Online
- ESP32 #2: 🟢 Online
- ESP32 #3: 🟢 Online

#### 6. Test Kontrol

Coba kontrol device dari website:
- Klik "Smart Lamp" → Klik "NYALA"
- Cek Serial Monitor ESP32 #1
- Harus muncul: `Message arrived [iot/lamp/control]: ON`

✅ **BERHASIL! ESP32 dan Website sudah terhubung!**

---

## 🔧 Troubleshooting MQTT Connection

### Problem 1: Website Status "Disconnected"

**Cek 1: Browser Console**
```
F12 → Console tab
Lihat error: "WebSocket connection to 'ws://broker.hivemq.com:8000/mqtt' failed"
```

**Solusi:**
1. Cek koneksi internet
2. Coba broker lain:

Edit `.env.local`:
```env
NEXT_PUBLIC_MQTT_BROKER=test.mosquitto.org
NEXT_PUBLIC_MQTT_PORT=8080
```

Restart website:
```bash
# Ctrl+C untuk stop
npm run dev
```

**Cek 2: Firewall**

Pastikan firewall tidak blok port 8000/8080.

Disable firewall sementara untuk test:
```powershell
# PowerShell as Admin
netsh advfirewall set allprofiles state off
```

Test, lalu enable lagi:
```powershell
netsh advfirewall set allprofiles state on
```

---

### Problem 2: ESP32 "Attempting MQTT connection...failed"

**Cek Serial Monitor:**
```
Attempting MQTT connection...failed, rc=-2
```

**Error Codes:**
- `rc=-2`: Network error (WiFi bermasalah)
- `rc=-4`: Timeout (broker tidak bisa diakses)
- `rc=5`: Authentication failed (tidak relevan untuk public broker)

**Solusi 1: Cek WiFi**
```
Pastikan ESP32 sudah connect WiFi:
- Lihat "WiFi connected" di Serial Monitor
- Lihat IP address muncul
```

**Solusi 2: Ganti Broker**

Edit ESP32 code:
```cpp
// Ganti dari HiveMQ
const char* mqtt_server = "broker.hivemq.com";

// Ke Mosquitto
const char* mqtt_server = "test.mosquitto.org";
```

Upload ulang.

**Solusi 3: Test Ping**

Dari komputer, test ping broker:
```bash
ping broker.hivemq.com
```

Jika tidak bisa ping, berarti broker/internet bermasalah.

---

### Problem 3: ESP32 Connect tapi Device Status "Offline"

**Penyebab:**
ESP32 connect ke MQTT tapi tidak kirim heartbeat.

**Solusi:**

Cek di Serial Monitor, pastikan muncul setiap 10 detik:
```
Publishing heartbeat...
```

Jika tidak muncul, cek code ESP32:
```cpp
// Pastikan ada di loop()
if (currentMillis - lastHeartbeat >= heartbeatInterval) {
  lastHeartbeat = currentMillis;
  client.publish("iot/esp32_1/heartbeat", "ONLINE");
}
```

---

### Problem 4: Kontrol dari Website Tidak Sampai ke ESP32

**Test dengan MQTT Explorer:**

1. Download: http://mqtt-explorer.com/
2. Install dan buka
3. Add connection:
   - Host: `broker.hivemq.com`
   - Port: `1883`
4. Connect
5. Publish manual ke topic: `iot/lamp/control` dengan message: `ON`
6. Cek Serial Monitor ESP32, harus muncul message

**Jika MQTT Explorer bisa tapi website tidak:**
- Cek browser console untuk error
- Pastikan website status "Connected"
- Refresh browser

**Jika MQTT Explorer juga tidak bisa:**
- Broker bermasalah
- Ganti broker atau install Mosquitto lokal

---

## 🚀 ALTERNATIF: Install Mosquitto Lokal (Lebih Cepat & Stabil)

Jika HiveMQ lambat atau tidak stabil, install Mosquitto di komputer.

### Langkah Install (Windows):

#### 1. Download & Install
```
1. Buka: https://mosquitto.org/download/
2. Download: mosquitto-2.0.18-install-windows-x64.exe
3. Install dengan default settings
4. Centang "Install as service"
```

#### 2. Konfigurasi

Buka Notepad **as Administrator**, edit:
```
C:\Program Files\mosquitto\mosquitto.conf
```

Tambahkan di akhir:
```conf
listener 1883
listener 9001
protocol websockets
allow_anonymous true
```

Save.

#### 3. Restart Service

PowerShell **as Administrator**:
```powershell
net stop mosquitto
net start mosquitto
```

#### 4. Allow Firewall

```powershell
netsh advfirewall firewall add rule name="Mosquitto MQTT" dir=in action=allow protocol=TCP localport=1883
netsh advfirewall firewall add rule name="Mosquitto WebSocket" dir=in action=allow protocol=TCP localport=9001
```

#### 5. Cek IP Komputer

```powershell
ipconfig
```

Catat IP (contoh: 192.168.1.100)

#### 6. Update Website

Edit `.env.local`:
```env
NEXT_PUBLIC_MQTT_BROKER=192.168.1.100
NEXT_PUBLIC_MQTT_PORT=9001
```

Restart website.

#### 7. Update ESP32

Edit **SEMUA** file .ino:
```cpp
const char* mqtt_server = "192.168.1.100";  // IP komputer kamu
const int mqtt_port = 1883;
```

Upload ulang ke semua ESP32.

#### 8. Test

Buka MQTT Explorer:
- Host: `localhost`
- Port: `1883`
- Connect

Jika berhasil, Mosquitto sudah jalan!

✅ **Mosquitto lokal lebih cepat (<10ms) dan tidak perlu internet!**

---

## 📊 Monitoring MQTT Traffic

### Cara 1: MQTT Explorer (Recommended)

**Setup:**
```
1. Download: http://mqtt-explorer.com/
2. Install
3. Add connection:
   - HiveMQ: broker.hivemq.com:1883
   - Mosquitto: localhost:1883
4. Connect
```

**Kegunaan:**
- Lihat semua topics real-time
- Monitor message yang dikirim/diterima
- Test publish/subscribe manual
- Debug connection issues

**Test Publish:**
1. Klik kanan di topic tree
2. Publish → Topic: `iot/lamp/control`, Message: `ON`
3. Cek Serial Monitor ESP32, harus muncul message

### Cara 2: Command Line (Mosquitto Tools)

**Subscribe (listen):**
```bash
mosquitto_sub -h broker.hivemq.com -t "iot/#" -v
```

**Publish (send):**
```bash
mosquitto_pub -h broker.hivemq.com -t "iot/lamp/control" -m "ON"
```

### Cara 3: Browser Console

Buka website → F12 → Console

Lihat log MQTT:
```
Connected to MQTT broker
Published to iot/lamp/control: ON
Message arrived [iot/lamp/status]: ON
```

---

## 🎯 Checklist Connection

### Website
- [ ] `npm run dev` jalan
- [ ] Browser buka http://localhost:3000
- [ ] Status MQTT: 🟢 Connected
- [ ] Tidak ada error di browser console

### ESP32
- [ ] WiFi credentials benar
- [ ] Serial Monitor: "WiFi connected"
- [ ] Serial Monitor: "MQTT connected"
- [ ] Heartbeat publish setiap 10 detik
- [ ] Device Status di website: 🟢 Online

### MQTT Broker
- [ ] HiveMQ: Bisa ping broker.hivemq.com
- [ ] Mosquitto: Service running (jika pakai lokal)
- [ ] MQTT Explorer bisa connect
- [ ] Tidak ada firewall blocking

### Test Komunikasi
- [ ] Kontrol dari website → ESP32 terima (cek Serial Monitor)
- [ ] ESP32 kirim data → Website terima (cek dashboard)
- [ ] MQTT Explorer bisa lihat traffic
- [ ] Latency < 1 detik

---

## 💡 Tips

### 1. Gunakan MQTT Explorer
Ini tool paling penting untuk debug MQTT. Install dan buka selalu saat development.

### 2. Monitor Serial Monitor
Buka Serial Monitor untuk semua ESP32 saat testing. Kamu bisa lihat semua message yang diterima.

### 3. Cek Browser Console
F12 → Console untuk lihat error MQTT di website.

### 4. Test Satu-Satu
Jangan langsung test semua ESP32. Test ESP32 #1 dulu sampai berhasil, baru ESP32 #2, dst.

### 5. Restart Semua
Jika ada masalah:
1. Restart website (Ctrl+C, npm run dev)
2. Restart ESP32 (tekan tombol RESET)
3. Restart router (jika perlu)

### 6. Gunakan Hotspot HP
Jika WiFi rumah bermasalah, pakai hotspot HP untuk testing.

### 7. Catat IP Address
Simpan IP komputer dan ESP32 untuk troubleshooting.

---

## 🆘 Quick Fix

Jika semua tidak jalan:

```bash
# 1. Stop website
Ctrl+C

# 2. Hapus cache
rm -rf .next node_modules
npm install

# 3. Restart website
npm run dev

# 4. Reset ESP32
Tekan tombol RESET di ESP32

# 5. Test dengan MQTT Explorer
Connect ke broker dan test publish/subscribe

# 6. Jika masih gagal, ganti broker
Edit .env.local dan ESP32 code, ganti ke test.mosquitto.org
```

---

## 📞 Bantuan

Jika masih stuck:

1. **Screenshot error** di Serial Monitor dan Browser Console
2. **Test dengan MQTT Explorer** - Apakah bisa connect?
3. **Cek ping broker** - Apakah bisa diakses?
4. **Coba broker lain** - test.mosquitto.org atau broker.emqx.io
5. **Install Mosquitto lokal** - Lebih stabil untuk development

---

## 🎉 Summary

**Cara Paling Mudah:**
1. Pakai HiveMQ public broker (sudah dikonfigurasi)
2. `npm run dev` untuk website
3. Upload ESP32 code dengan WiFi credentials
4. Tunggu 30 detik
5. Cek Device Status → 🟢 Online
6. Done!

**Jika Bermasalah:**
1. Install MQTT Explorer untuk monitoring
2. Cek Serial Monitor untuk error
3. Cek Browser Console untuk error
4. Ganti broker atau install Mosquitto lokal

**Performa Terbaik:**
- Install Mosquitto lokal
- Latency <10ms
- Tidak perlu internet
- Lebih stabil

Good luck! 🚀
