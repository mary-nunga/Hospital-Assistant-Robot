int irPin = 34;         // IR sensor on IO 34
int threshold = 200;    

bool isItemOnTray() {
  long sum = 0;

  // take 10 samples to smooth out noise
  for (int i = 0; i < 10; i++) {
    sum += analogRead(irPin);
    delay(5);
  }

  int value = sum / 10;

  Serial.print("IR value: ");
  Serial.print(value);

  bool item = (value >= threshold);

  if (item) {
    Serial.println("  -> ITEM ON TRAY");
  } else {
    Serial.println("  -> TRAY EMPTY");
  }

  return item;  // true = item present, false = empty
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  bool itemPresent = isItemOnTray();

  // later you can use itemPresent in your robot logic
  // e.g. if (!itemPresent) mark as delivered

  delay(300);
}
