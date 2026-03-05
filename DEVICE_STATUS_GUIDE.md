# 📡 Panduan Device Status & WiFi Pairing

## Fitur Baru

### 1. Device Status Monitoring
Dashboard sekarang menampilkan status real-time untuk setiap ESP32:
- 🟢 **Online**: Device terhubung dan mengirim heartbeat
- 🔴 **Offline**: Device tidak terhubung atau mati

### 2. Heartbeat System
Setiap ESP32 mengirim heartbeat setiap 10 detik ke topic:
- `iot/esp32_1/heartbeat` → "ONLINE"
- `iot/esp32_2/heartbeat` → "ONLINE"

Jika tidak ada heartbeat selama 30 detik, device dianggap offline.

### 3. WiFi Pairing Mode
Fitur untuk mengkonfigurasi WiFi ESP32 dari dashboard tanpa perlu upload code ulang.

## Cara Menggunakan

### Melihat Status Device

1. Buka dashboard di http://localhost:3000
2. Klik menu **"Device Status"** di sidebar
3. Lihat status setiap ESP32:
   - Indikator hijau berkedip = Online
   - Indikator merah = Offline
   - Timestamp terakhir terlihat

### WiFi Pairing

1. **Persiapan ESP32**
   - Pastikan ESP32 sudah upload code terbaru
   - Tekan tombol BOOT selama 3 detik (opsional, untuk mode pairing)

2. **Dari Dashboard**
   - Klik menu "Device Status"
   - Klik tombol "WiFi Pairing Mode"
   - Masukkan SSID WiFi
   - Masukkan Password WiFi
   - Pilih "Kirim ke ESP32 #1" atau "Kirim ke ESP32 #2"

3. **ESP32 akan:**
   - Menerima konfigurasi via MQTT
   - Restart dan connect ke WiFi baru
   - Mulai kirim heartbeat

## MQTT Topics Baru

### Heartbeat
```
Topic: iot/esp32_1/heartbeat
Message: "ONLINE"
Interval: 10 detik
```

```
Topic: iot/esp32_2/heartbeat
Message: "ONLINE"
Interval: 10 detik
```

### WiFi Configuration
```
Topic: iot/esp32_1/wifi/config
Message: {"ssid":"NamaWiFi","password":"PasswordWiFi"}
```

```
Topic: iot/esp32_2/wifi/config
Message: {"ssid":"NamaWiFi","password":"PasswordWiFi"}
```

## Notifikasi

Dashboard akan menampilkan notifikasi otomatis jika:
- ESP32 #1 offline
- ESP32 #2 offline
- Koneksi MQTT terputus

## Troubleshooting

### Device Selalu Offline
1. Cek koneksi WiFi ESP32
2. Pastikan MQTT broker accessible
3. Cek Serial Monitor untuk error
4. Pastikan code terbaru sudah di-upload

### Heartbeat Tidak Terkirim
1. Cek topic MQTT di Serial Monitor
2. Pastikan interval heartbeat tidak terlalu cepat
3. Cek koneksi MQTT broker

### WiFi Pairing Tidak Bekerja
1. Pastikan ESP32 subscribe ke topic wifi/config
2. Cek Serial Monitor untuk melihat message diterima
3. Implementasi EEPROM save jika perlu persistent config

## Update ESP32 Code

Jangan lupa upload ulang code ESP32 yang sudah diupdate:
- `esp32-code/ESP32_1_Lamp_Gas_Feeder.ino`
- `esp32-code/ESP32_2_Trash_Clothesline.ino`

Kedua file sudah include:
- Heartbeat system
- WiFi config receiver
- Subscribe ke topic baru

## Tips

1. **Monitoring Jarak Jauh**
   - Gunakan MQTT broker cloud (HiveMQ, CloudMQTT)
   - Akses dashboard dari mana saja

2. **Backup Configuration**
   - Simpan WiFi credentials di EEPROM ESP32
   - Auto-reconnect jika WiFi terputus

3. **Multiple ESP32**
   - Mudah scale ke lebih banyak device
   - Tambah heartbeat topic baru untuk ESP32 #3, #4, dst

4. **Alert System**
   - Bisa tambah notifikasi email/telegram
   - Alert jika device offline lebih dari X menit
