#include <ESP32Servo.h>

Servo doorServo;
Servo gateServoLeft;
Servo gateServoRight;

// Pin baru yang lebih aman
#define DOOR_SERVO_PIN 2      // Ganti dari 13
#define GATE_LEFT_PIN 4       // Ganti dari 25
#define GATE_RIGHT_PIN 5      // Ganti dari 26

void setup() {
  Serial.begin(115200);
  Serial.println("Testing with NEW PINS");
  
  doorServo.attach(DOOR_SERVO_PIN);
  gateServoLeft.attach(GATE_LEFT_PIN);
  gateServoRight.attach(GATE_RIGHT_PIN);
  
  Serial.println("Door: GPIO 2, Gate L: GPIO 4, Gate R: GPIO 5");
  delay(1000);
}

void loop() {
  Serial.println("Moving all servos to 0°");
  doorServo.write(0);
  gateServoLeft.write(0);
  gateServoRight.write(90);
  delay(3000);
  
  Serial.println("Moving all servos to 90°");
  doorServo.write(90);
  gateServoLeft.write(90);
  gateServoRight.write(0);
  delay(3000);
}