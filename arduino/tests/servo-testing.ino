#include <ESP32Servo.h>

Servo sprayServo;  

void setup() {
  // attach servo signal 
  sprayServo.attach(26);
}

void loop() {
  // left
  sprayServo.write(0);
  delay(1000);

  // center
  sprayServo.write(90);
  delay(1000);

  // right
  sprayServo.write(180);
  delay(1000);

  // back to center so it doesn’t stay at extremes
  sprayServo.write(90);
  delay(1000);
}
