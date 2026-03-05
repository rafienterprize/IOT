# 🚀 Panduan Setup Lengkap IoT Dashboard

## 📋 Yang Kamu Butuhkan

### Software
- Node.js 18+ (download dari nodejs.org)
- Arduino IDE (untuk upload code ke ESP32)
- Browser modern (Chrome/Firefox/Edge)

### Hardware
- 2x ESP32 Development Board
- Sensor dan komponen (lihat esp32-code/README_ESP32.md)
- Kabel USB untuk upload code
- WiFi router

## 🎯 Langkah Setup

### STEP 1: Setup Website Next.js

1. **Install dependencies**
   ```bash
   npm install
   ```

2. **Konfigurasi MQTT Broker**
   
   Edit file `.env.local`:
   ```env
   NEXT_PUBLIC_MQTT_BROKER=broker.hivemq.com
   NEXT_PUBLIC_MQTT_PORT=8000
   ```

3. **Jalankan development server**
   ```bash
   npm run dev
   ```

4. **Buka browser**
   
   Akses: http://localhost:3000

### STEP 2: Setup ESP32

1. **Install Arduino IDE**
   - Download dari: https://www.arduino.cc/en/software
   - Install ESP32 board support (lihat esp32-code/README_ESP32.md)

2. **Install Library**
   
   Di Arduino IDE, install:
   - PubSubClient
   - ESP32Servo

3. **Upload Code ESP32 #1**
   
   File: `esp32-code/ESP32_1_Lamp_Gas_Feeder.ino`
   
   Edit WiFi credentials:
   ```cpp
   const char* ssid = "NAMA_WIFI_KAMU";
   const char* password = "PASSWORD_WIFI_KAMU";
   ```
   
   Upload ke ESP32 pertama.

4. **Upload Code ESP32 #2**
   
   File: `esp32-code/ESP32_2_Trash_Clothesline.ino`
   
   Edit WiFi credentials (sama seperti ESP32 #1).
   
   Upload ke ESP32 kedua.

### STEP 3: Wiring Hardware

Lihat diagram lengkap di `esp32-code/README_ESP32.md`

**ESP32 #1 - Quick Reference:**
- GPIO 2 → Relay (Lamp)
- GPIO 34 → Gas Sensor
- GPIO 18 → Servo (Fish Feeder)

**ESP32 #2 - Quick Reference:**
- GPIO 5, 18 → Ultrasonic (Trash)
- GPIO 34 → Rain Sensor
- GPIO 25, 26 → Motor Driver (Clothesline)

### STEP 4: Testing

1. **Cek Serial Monitor**
   
   Buka Serial Monitor (115200 baud) untuk setiap ESP32.
   
   Pastikan muncul:
   ```
   WiFi connected
   IP address: 192.168.x.x
   MQTT connected
   ```

2. **Test dari Website**
   
   - Klik tombol Smart Lamp → Cek LED nyala/mati
   - Monitor grafik Gas Detector
   - Test Fish Feeder
   - Cek Smart Trash level
   - Test Jemuran buka/tutup

## 🔧 Konfigurasi Lanjutan

### Menggunakan MQTT Broker Lokal

Jika ingin lebih cepat dan tidak bergantung internet:

1. **Install Mosquitto**
   
   Windows: Download dari mosquitto.org
   
   Linux:
   ```bash
   sudo apt-get install mosquitto
   sudo systemctl start mosquitto
   ```

2. **Update Konfigurasi**
   
   Di `.env.local`:
   ```env
   NEXT_PUBLIC_MQTT_BROKER=192.168.1.XXX  # IP komputer kamu
   NEXT_PUBLIC_MQTT_PORT=1883
   ```
   
   Di ESP32 code:
   ```cpp
   const char* mqtt_server = "192.168.1.XXX";
   ```

### Deploy ke Production

1. **Build website**
   ```bash
   npm run build
   npm start
   ```

2. **Deploy ke Vercel (Gratis)**
   ```bash
   npm install -g vercel
   vercel deploy
   ```

## 🐛 Troubleshooting

### Website tidak connect ke MQTT
- Cek browser console (F12)
- Pastikan MQTT broker accessible
- Coba broker alternatif: test.mosquitto.org

### ESP32 tidak connect WiFi
- Pastikan WiFi 2.4GHz (bukan 5GHz)
- Cek SSID dan password
- Restart ESP32

### Sensor tidak terbaca
- Cek wiring
- Test dengan code sederhana
- Cek voltage (3.3V vs 5V)

### Data tidak muncul di dashboard
- Cek Serial Monitor ESP32
- Pastikan topic MQTT sama
- Refresh browser

## 📱 Akses dari HP

1. Cek IP komputer yang menjalankan website:
   ```bash
   ipconfig  # Windows
   ifconfig  # Linux/Mac
   ```

2. Dari HP (harus satu WiFi), buka:
   ```
   http://192.168.1.XXX:3000
   ```

## 🎨 Customization

### Ubah Warna/Tema
Edit `app/globals.css` dan komponen di `components/`

### Tambah Device Baru
1. Buat komponen baru di `components/`
2. Tambahkan di `app/page.tsx`
3. Update ESP32 code dengan topic baru

### Ubah Interval Sensor
Di ESP32 code, edit:
```cpp
const long gasInterval = 2000;  // 2 detik
const long trashInterval = 5000;  // 5 detik
```

## 📊 Monitoring

### Lihat Log MQTT
Gunakan MQTT Explorer (download gratis) untuk monitor semua messages.

### Debug ESP32
Serial Monitor akan menampilkan semua aktivitas.

## 🔐 Security Tips

Untuk production:
- Gunakan MQTT dengan username/password
- Enable SSL/TLS
- Gunakan private broker
- Jangan expose port ke internet

## 💡 Tips & Tricks

1. **Power Supply**: Gunakan power supply terpisah untuk motor/servo
2. **Stabilitas**: Tambahkan capacitor di power line
3. **Range**: Gunakan ESP32 dengan antena eksternal untuk jangkauan lebih jauh
4. **Backup**: Simpan konfigurasi WiFi di EEPROM
5. **OTA Update**: Implementasi OTA untuk update ESP32 tanpa kabel

## 📞 Bantuan

Jika ada masalah:
1. Cek dokumentasi di README.md
2. Lihat esp32-code/README_ESP32.md
3. Cek Serial Monitor untuk error messages
4. Test satu device dulu sebelum gabungkan semua

Selamat mencoba! 🚀
