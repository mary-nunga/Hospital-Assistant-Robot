int relayPin = 27;

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // off
}

void loop() {
  digitalWrite(relayPin, HIGH); // turn pump ON
  delay(2000);
  digitalWrite(relayPin, LOW);  // turn pump OFF
  delay(2000);
}
