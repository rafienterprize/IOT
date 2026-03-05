# 🏠 IoT Monitoring Dashboard

Dashboard monitoring dan kontrol IoT menggunakan Next.js 14 dan MQTT untuk ESP32.

## 🚀 Fitur

1. **Smart Lamp** - Kontrol lampu ON/OFF + Timer otomatis
2. **Gas Detector** - Monitoring gas dengan grafik real-time
3. **Fish Feeder** - Pemberi makan ikan otomatis dengan jadwal
4. **Smart Trash Bin** - Monitor level sampah
5. **Smart Clothesline** - Jemuran otomatis dengan deteksi hujan

## 📦 Instalasi

```bash
npm install
```

## 🔧 Konfigurasi

Edit file `.env.local` untuk konfigurasi MQTT broker:

```env
NEXT_PUBLIC_MQTT_BROKER=broker.hivemq.com
NEXT_PUBLIC_MQTT_PORT=8000
```

## 🏃 Menjalankan

```bash
npm run dev
```

Buka [http://localhost:3000](http://localhost:3000)

## 📡 MQTT Topics

### Smart Lamp
- `iot/lamp/control` - Publish: ON/OFF
- `iot/lamp/status` - Subscribe: ON/OFF
- `iot/lamp/timer` - Publish: {"on":"08:00","off":"22:00"}

### Gas Detector
- `iot/gas/level` - Subscribe: nilai PPM (float)

### Fish Feeder
- `iot/feeder/control` - Publish: FEED
- `iot/feeder/status` - Subscribe: FED
- `iot/feeder/schedule` - Publish: ["08:00","18:00"]

### Smart Trash
- `iot/trash/level` - Subscribe: 0-100 (%)

### Smart Clothesline
- `iot/clothesline/control` - Publish: OPEN/CLOSE
- `iot/clothesline/status` - Subscribe: OPEN/CLOSE
- `iot/clothesline/rain` - Subscribe: RAIN/CLEAR
- `iot/clothesline/auto` - Publish: ON/OFF

## 🔌 Koneksi ESP32

### Hardware yang Dibutuhkan
- 2x ESP32
- Relay modules
- Sensor gas (MQ-2/MQ-135)
- Sensor hujan
- Sensor ultrasonik (HC-SR04) untuk trash bin
- Servo motor untuk fish feeder
- LED untuk smart lamp

### ESP32 #1 - Smart Lamp, Gas Detector, Fish Feeder
### ESP32 #2 - Smart Trash, Smart Clothesline

Lihat folder `esp32-code/` untuk kode Arduino lengkap.

## 🌐 Deploy

```bash
npm run build
npm start
```

Atau deploy ke Vercel:
```bash
vercel deploy
```
