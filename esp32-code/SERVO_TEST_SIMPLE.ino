#include <ESP32Servo.h>

Servo doorServo;
Servo gateServoLeft;
Servo gateServoRight;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Servo Test");
  
  // Attach servos
  doorServo.attach(13);
  gateServoLeft.attach(25);
  gateServoRight.attach(26);
  
  Serial.println("Servos attached");
  delay(1000);
}

void loop() {
  Serial.println("Testing Door Servo (GPIO 13)...");
  doorServo.write(0);
  delay(2000);
  doorServo.write(90);
  delay(2000);
  
  Serial.println("Testing Gate Left Servo (GPIO 25)...");
  gateServoLeft.write(0);
  delay(2000);
  gateServoLeft.write(90);
  delay(2000);
  
  Serial.println("Testing Gate Right Servo (GPIO 26)...");
  gateServoRight.write(90);
  delay(2000);
  gateServoRight.write(0);
  delay(2000);
  
  Serial.println("All servos tested. Repeating...");
  delay(1000);
}