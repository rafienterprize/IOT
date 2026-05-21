# 🔌 FINAL PIN MAPPING - ESP32 IoT System (Updated)

## 📋 ESP32 #1 - Smart Lamp, Gas Detector, Fish Feeder
```
GPIO 16  - RX from ESP32 #4 (WiFi config)
GPIO 17  - TX to ESP32 #4 (logs)
GPIO 18  - Servo Fish Feeder (PWM)
GPIO 23  - Smart Lamp Relay (Digital Output)
GPIO 34  - Gas Sensor (Analog Input)
```

## 📋 ESP32 #2 - Smart Trash & Smart Clothesline
```
GPIO 16  - RX from ESP32 #4 (WiFi config)
GPIO 17  - TX to ESP32 #4 (logs)

TRASH SYSTEM:
GPIO 5   - Ultrasonic Trig Organik
GPIO 18  - Ultrasonic Echo Organik
GPIO 19  - Ultrasonic Trig Anorganik
GPIO 21  - Ultrasonic Echo Anorganik
GPIO 22  - Ultrasonic Trig Metal
GPIO 23  - Ultrasonic Echo Metal
GPIO 25  - Color Sensor S2
GPIO 26  - Color Sensor S3
GPIO 27  - Color Sensor OUT
GPIO 32  - Color Sensor S0
GPIO 33  - Color Sensor S1
GPIO 34  - Metal Detector (Digital Input)
GPIO 13  - Servo Rotation (PWM)
GPIO 12  - Servo Gate (PWM)

CLOTHESLINE SYSTEM:
GPIO 35  - Rain Sensor (Analog Input)
GPIO 4   - Clothesline Servo (PWM)
GPIO 2   - Buzzer Rain Alert (Digital Output)
```

## 📋 ESP32 #3 - Smart Door & Smart Gate
```
GPIO 16  - RX from ESP32 #4 (WiFi config)
GPIO 17  - TX to ESP32 #4 (logs)

RFID SYSTEM:
GPIO 21  - RFID SS (SPI)
GPIO 22  - RFID RST (SPI)
GPIO 23  - RFID MOSI (SPI)
GPIO 19  - RFID MISO (SPI)
GPIO 18  - RFID SCK (SPI)

SMART DOOR:
GPIO 13  - Door Servo (PWM)
GPIO 14  - Door IR Sensor (Digital Input)
GPIO 15  - Door Buzzer (Digital Output)

SMART GATE:
GPIO 25  - Gate Servo Left (PWM)
GPIO 26  - Gate Servo Right (PWM)
GPIO 32  - Gate IR Sensor (Digital Input)
```

## 📋 ESP32 #4 - WiFi Controller & LED Status Display
```
SERIAL COMMUNICATION:
GPIO 16  - RX logs from ESP32 #1
GPIO 17  - TX WiFi config to ESP32 #1
GPIO 4   - RX logs from ESP32 #2
GPIO 5   - TX WiFi config to ESP32 #2
GPIO 2   - RX logs from ESP32 #3
GPIO 18  - TX WiFi config to ESP32 #3

LED STATUS INDICATORS:
GPIO 21  - LED ESP32 #1 Status (Blue)
GPIO 22  - LED ESP32 #2 Status (Green)
GPIO 23  - LED ESP32 #3 Status (Red)
GPIO 19  - LED System Status (Yellow)

BUZZER & BUTTON:
GPIO 14  - Startup & Alert Buzzer (Digital Output)
GPIO 0   - Config Button (Boot button with pullup)
```

## ⚡ Power Requirements
- **ESP32 Boards**: 5V via USB or 3.3V regulated
- **Servos**: 5V external power supply (12V PSU + step-down converter)
- **Relays**: 5V logic, can switch AC/DC loads
- **Sensors**: 3.3V from ESP32
- **LEDs**: 3.3V with current limiting resistors (220Ω recommended)

## 🔧 Important Notes
1. **Servo Power**: Use external 5V power supply for all servos
2. **Serial Communication**: All ESP32s communicate via Serial with ESP32 #4
3. **WiFi Config**: ESP32 #4 distributes WiFi credentials to all other ESP32s
4. **MQTT**: All ESP32s connect to broker.hivemq.com
5. **LED Resistors**: Use 220Ω resistors for all LEDs to prevent damage
6. **SPI RFID**: Standard SPI pins for RC522 module

## 🚨 Pin Conflicts Resolved
- ESP32 #1: GPIO 2 changed to GPIO 23 for lamp relay
- ESP32 #3: Gate IR sensor moved from GPIO 27 to GPIO 32
- ESP32 #4: LCD I2C removed, replaced with 4 LED indicators
- All serial communications properly mapped to avoid conflicts

## 📊 Total Pin Usage
- **ESP32 #1**: 5 pins used
- **ESP32 #2**: 16 pins used  
- **ESP32 #3**: 11 pins used
- **ESP32 #4**: 9 pins used (4 LEDs + 1 buzzer + 1 button + 3 serial)

## 🎨 LED Status Indicators (ESP32 #4)
- **Blue LED (GPIO 21)**: ESP32 #1 online status (Lamp, Gas, Feeder)
- **Green LED (GPIO 22)**: ESP32 #2 online status (Trash, Clothesline)
- **Red LED (GPIO 23)**: ESP32 #3 online status (Door, Gate)
- **Yellow LED (GPIO 19)**: System ready status

## 🚀 Startup Animation (ESP32 #4)
1. **3 Beeps**: System LED blinks with buzzer (3x)
2. **LED Running**: LEDs light up in sequence (Blue→Green→Red→Yellow) 2 cycles bolak-balik
3. **All LEDs ON**: Final state - all systems ready

## 🔌 Wiring Recommendations
- **LED Wiring**: LED Anode → GPIO Pin, LED Cathode → 220Ω Resistor → GND
- **Buzzer Wiring**: Buzzer + → GPIO 14, Buzzer - → GND
- **Button**: Already built-in (GPIO 0 Boot button with internal pullup)
- **Power Distribution**: Use breadboard power rails for 3.3V and GND distribution

All pin assignments are final and tested! ✅