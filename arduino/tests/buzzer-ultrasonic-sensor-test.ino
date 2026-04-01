
// Pins 
const int trigPin   = 13;   // Ultrasonic TRIG
const int echoPin   = 14;   // Ultrasonic ECHO
const int buzzerPin = 33;   // Buzzer (from Acebott sample code)

// Distance threshold in cm
const float ALERT_DISTANCE = 20.0;   // beep if object is closer than this

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(trigPin, LOW);
  noTone(buzzerPin);  // make sure buzzer is off at start

  Serial.println("Ultrasonic + Buzzer test started...");
}

void loop() {
  // --- Trigger the ultrasonic pulse ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // --- Read echo pulse duration ---
  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms

  float distance_cm;

  if (duration == 0) {
    // No echo received within timeout
    distance_cm = 0;
  } else {
    // Convert duration  to distance (cm)
    distance_cm = duration / 58.0;  
  }

  // --- Print distance to Serial Monitor ---
  Serial.print("Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");

  // --- Buzzer logic ---
  if (distance_cm > 0 && distance_cm < ALERT_DISTANCE) {
    // Object is close → beep at 2kHz
    tone(buzzerPin, 2000);
  } else {
    // No object close or invalid reading → silent
    noTone(buzzerPin);
  }

  delay(200); // small delay so serial is readable (5 readings/sec)
}
