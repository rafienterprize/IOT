# 🗑️ Smart Trash Sorting System - Wiring Guide

## Komponen yang Dibutuhkan

### Sensor & Actuator
- 3x Sensor Ultrasonik HC-SR04 (untuk level detection)
- 1x TCS3200 Color Sensor (untuk deteksi warna)
- 1x Metal Detector Sensor (Inductive Proximity Sensor)
- 2x Servo Motor (1 untuk rotasi bin, 1 untuk gate)
- 3x Tempat sampah (Organik, Anorganik, Metal)

### Mekanisme
- Platform putar (rotating platform) dengan servo
- Gate/pintu servo untuk drop sampah

## Pin Connections ESP32

### Sensor Ultrasonik (3 unit)

**Bin Organik:**
```
TRIG → GPIO 5
ECHO → GPIO 18
VCC → 5V
GND → GND
```

**Bin Anorganik:**
```
TRIG → GPIO 19
ECHO → GPIO 21
VCC → 5V
GND → GND
```

**Bin Metal:**
```
TRIG → GPIO 22
ECHO → GPIO 23
VCC → 5V
GND → GND
```

### Color Sensor TCS3200

```
S0 → GPIO 32
S1 → GPIO 33
S2 → GPIO 25
S3 → GPIO 26
OUT → GPIO 27
VCC → 3.3V
GND → GND
```

### Metal Detector Sensor

```
OUT → GPIO 34
VCC → 5V
GND → GND
```

### Servo Motors

**Rotation Servo (untuk putar platform):**
```
Signal → GPIO 13
VCC → 5V (external power recommended)
GND → GND
```

**Gate Servo (untuk buka/tutup pintu):**
```
Signal → GPIO 12
VCC → 5V (external power recommended)
GND → GND
```

## Cara Kerja Sistem

### 1. Detection Phase
```
Sampah masuk → Sensor warna scan → Metal detector check
```

### 2. Classification
- **Metal Detected** → Kategori: Metal (240°)
- **Warna Hijau/Coklat** → Kategori: Organik (0°)
- **Warna Lain** → Kategori: Anorganik (120°)

### 3. Sorting Phase
```
Rotasi platform ke posisi → Buka gate → Drop sampah → Tutup gate
```

### 4. Monitoring
- Sensor ultrasonik di setiap bin monitor level
- Data dikirim ke dashboard via MQTT

## Posisi Rotasi

```
     0° (Organik)
        |
        |
240° ---|--- 120°
(Metal)     (Anorganik)
```

## MQTT Topics

### Subscribe (ESP32 menerima)
```
iot/trash/rotate         → Manual rotation (0, 120, 240)
iot/trash/reset          → Reset ke posisi 0°
```

### Publish (ESP32 mengirim)
```
iot/trash/organik/level     → Level bin organik (0-100%)
iot/trash/anorganik/level   → Level bin anorganik (0-100%)
iot/trash/metal/level       → Level bin metal (0-100%)
iot/trash/detection         → {"type":"Organik","rotation":0}
iot/trash/rotation          → Current rotation angle
```

## Kalibrasi

### Color Sensor
1. Letakkan kertas putih → catat nilai RGB
2. Letakkan kertas hitam → catat nilai RGB
3. Test dengan sampah organik (hijau/coklat)
4. Test dengan sampah anorganik (plastik warna-warni)
5. Adjust threshold di code

### Metal Detector
1. Test dengan besi/kaleng
2. Adjust sensitivity potentiometer
3. Pastikan jarak deteksi 2-5cm

### Servo Rotation
1. Test posisi 0° → harus align dengan bin Organik
2. Test posisi 120° → harus align dengan bin Anorganik
3. Test posisi 240° → harus align dengan bin Metal
4. Adjust angle di code jika perlu

## Tips & Troubleshooting

### Servo Tidak Kuat
- Gunakan power supply eksternal 5V 2A
- Jangan power servo dari ESP32 langsung
- Sambungkan GND power supply dengan GND ESP32

### Color Sensor Tidak Akurat
- Pastikan pencahayaan konsisten
- Gunakan LED putih untuk illumination
- Kalibrasi ulang dengan berbagai sampel

### Metal Detector Terlalu Sensitif
- Adjust potentiometer di sensor
- Jauhkan dari metal lain
- Test dengan berbagai jenis metal

### Rotasi Tidak Presisi
- Gunakan servo dengan torque tinggi (MG996R)
- Pastikan platform tidak terlalu berat
- Tambahkan delay setelah rotasi

## Upgrade Ideas

1. **Tambah Sensor Berat** - Timbang sampah sebelum sort
2. **Camera Vision** - Gunakan ESP32-CAM untuk AI classification
3. **Compactor** - Tambah mekanisme press sampah
4. **Multi-Level** - Sistem bertingkat untuk kapasitas lebih besar
5. **Auto Bag Change** - Deteksi penuh dan alert ganti kantong

## Safety

⚠️ **PENTING:**
- Jangan masukkan tangan saat sistem beroperasi
- Pastikan gate servo tidak mencubit
- Gunakan power supply yang aman
- Test manual dulu sebelum auto mode
