#include <ESP32Servo.h>

Servo testServo;

void setup() {
  Serial.begin(115200);
  Serial.println("Single Servo Test");
  
  // Test hanya 1 servo di pin yang pasti aman
  testServo.attach(2);
  Serial.println("Servo attached to GPIO 2");
  delay(1000);
}

void loop() {
  Serial.println("Moving to 0°");
  testServo.write(0);
  delay(2000);
  
  Serial.println("Moving to 45°");
  testServo.write(45);
  delay(2000);
  
  Serial.println("Moving to 90°");
  testServo.write(90);
  delay(2000);
  
  Serial.println("Moving to 135°");
  testServo.write(135);
  delay(2000);
  
  Serial.println("Moving to 180°");
  testServo.write(180);
  delay(2000);
}