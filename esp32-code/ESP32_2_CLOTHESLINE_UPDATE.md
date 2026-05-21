# ESP32 #2 Clothesline Update - Servo + Raindrop Sensor

## Updated Pin Configuration:

| Komponen | Pin ESP32 | Keterangan |
|----------|-----------|------------|
| **Rain Sensor** | GPIO 35 | Input Analog - Sensor hujan |
| **Clothesline Servo** | GPIO 4 | Output PWM - Servo jemuran |
| **Clothesline Buzzer** | GPIO 2 | Output Digital - Alert hujan |

## Logic Flow:

### **Raindrop Sensor:**
- **Analog Value < 2000** = Ada air (hujan terdeteksi)
- **Analog Value >= 2000** = Kering (tidak hujan)

### **Servo Clothesline:**
- **90°** = Jemuran BUKA (pakaian terbuka)
- **0°** = Jemuran TUTUP (pakaian terlindung)

### **Auto Mode Logic:**
1. **Rain Detected** (sensor < 2000):
   - **Buzzer alert**: 3 beeps (200ms on/off)
   - Servo bergerak dari 90° → 0° (tutup jemuran)
   - Publish: `iot/clothesline/rain` → "RAIN"
   - Status: "Clothesline: CLOSED"

2. **Rain Stopped** (sensor >= 2000):
   - Publish: `iot/clothesline/rain` → "CLEAR"
   - Optional: Auto buka jemuran (disabled by default)

### **Manual Control:**
- **MQTT Topic**: `iot/clothesline/control`
- **OPEN**: Servo ke 90° (buka jemuran)
- **CLOSE**: Servo ke 0° (tutup jemuran)

## Wiring Diagram:

```
ESP32 #2
├── Rain Sensor
│   ├── VCC → 3.3V
│   ├── GND → GND
│   └── AO → GPIO 35 (Analog)
│
├── Clothesline Servo
│   ├── VCC (Red) → 5V External
│   ├── GND (Black) → Common GND
│   └── Signal (Yellow) → GPIO 4
│
└── Buzzer
    ├── (+) → GPIO 2
    └── (-) → GND
```

## Sensor Specifications:

### **Raindrop Sensor:**
- **Type**: Analog rain detection sensor
- **Output**: 0-4095 (12-bit ADC)
- **Threshold**: < 2000 = wet, >= 2000 = dry
- **Response Time**: < 1 second

### **Servo Motor:**
- **Type**: SG90 or MG996R
- **Voltage**: 5V (external power recommended)
- **Angle Range**: 0° - 180°
- **Control**: PWM signal

### **Buzzer:**
- **Type**: Active buzzer 5V
- **Pattern**: 3 beeps saat hujan terdeteksi
- **Duration**: 200ms ON, 200ms OFF
- **Total Alert Time**: ~1.2 seconds

## MQTT Topics:

### **Status:**
- `iot/clothesline/status` → "OPEN" / "CLOSED"
- `iot/clothesline/rain` → "RAIN" / "CLEAR"
- `iot/esp32_2/status` → "Clothesline: OPEN/CLOSED"

### **Control:**
- `iot/clothesline/control` → "OPEN" / "CLOSE"
- `iot/clothesline/auto` → "ON" / "OFF"

## Changes from Previous Version:
- **Motor DC** → **Servo Motor** (GPIO 4)
- **Improved rain logic** with status change detection
- **Cleaner servo control** (instant movement)
- **Better logging** with rain status messages

## Testing:
1. **Simulate rain**: Tetes air ke sensor
2. **Check servo**: Harus bergerak dari 90° ke 0°
3. **Dry sensor**: Lap kering sensor
4. **Manual control**: Test via MQTT atau website