#include <LiquidCrystal.h>
#include <Servo.h>

//  LCD pins 
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

//  Pins 
const int BTN_PIN   = 2;   // start button
const int TRIG_PIN  = 3;   // ultrasonic trig
const int ECHO_PIN  = 4;   // ultrasonic echo
const int SERVO_PIN = 5;   // spray nozzle servo
const int PUMP_PIN  = 6;   // LED for pump
const int BUZZER    = 13;   // buzzer
const int LDR_PIN  = A0;   // IR tray sensor 

//  Objects 
Servo sprayServo;

//Code settings 
const int   OBSTACLE_CM      = 20;    // stop/reroute if obstacle ≤ this
const int   BACKUP_TIME_MS   = 800;   // reroute time
const int   SWEEP_MIN        = 45;    // servo sweep range
const int   SWEEP_MAX        = 135;
const int   SWEEP_STEP       = 5;
const int   SWEEP_DELAY_MS   = 100;   // servo sweep speed
const long  SPRAY_MS         = 5000;  // pump ON duration (5s)
const long  PICKUP_TIMEOUTMS = 8000;  // wait for tray to become empty
int        LDR_EMPTY_THRESHOLD = 600;  

//Distance helper
long getDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 60000);  // timeout ~60ms
  if (duration == 0) return 999;
  long dist = duration / 58;

  // Display distance for debugging
  lcd.setCursor(0, 1);
  lcd.print("D:");
  lcd.print(dist);
  lcd.print("cm   ");

  return dist;
}

//Obstacle check 
bool obstacleAhead() {
  long d = getDistanceCm();
  return (d <= OBSTACLE_CM);
}

bool trayEmpty() {
  int v = analogRead(LDR_PIN);               // 0..1023
  lcd.setCursor(9, 1); lcd.print("L:"); lcd.print(v); lcd.print("   ");
  return v >= LDR_EMPTY_THRESHOLD;           // bright == empty
}

//Tray pickup check 
bool waitForPickup() {
  unsigned long start = millis();
  while (millis() - start < PICKUP_TIMEOUTMS) {
    if (trayEmpty()) return true;            // item taken
    delay(100);
  }
  return false; // timeout
}

//Spray routine
void sprayRoutine() {
  lcd.clear();
  lcd.print("Disinfecting...");
  digitalWrite(PUMP_PIN, HIGH); // pump ON
  tone(BUZZER, 1500, 200);      // short tone start

  unsigned long t0 = millis();
  while (millis() - t0 < SPRAY_MS) {
    for (int a = SWEEP_MIN; a <= SWEEP_MAX; a += SWEEP_STEP) {
      sprayServo.write(a);
      delay(SWEEP_DELAY_MS);
    }
    for (int a = SWEEP_MAX; a >= SWEEP_MIN; a -= SWEEP_STEP) {
      sprayServo.write(a);
      delay(SWEEP_DELAY_MS);
    }
  }

  digitalWrite(PUMP_PIN, LOW);  // pump OFF
  lcd.clear();
  lcd.print("Spray complete");
  tone(BUZZER, 1000, 800);      // long tone alert
  delay(2000);
}

//Setup
void setup() {
  lcd.begin(16, 2);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  sprayServo.attach(SERVO_PIN);
  sprayServo.write(90);

  lcd.print("Press button");
  delay(500);
}

//main code
void loop() {
  if (digitalRead(BTN_PIN) == LOW) {  // pressed (pull-up logic)
    lcd.clear();
    lcd.print("Delivery en route");
    delay(3000);

    //obstacle loop
    while (true) {
      if (obstacleAhead()) {
        lcd.clear(); lcd.print("Obstacle! Stop");
        tone(BUZZER, 700, 200); // alert beep
        delay(800);

        lcd.clear(); lcd.print("Rerouting...");
        delay(BACKUP_TIME_MS);
      } else {
        break;
      }
      delay(150);
    }
    //Arrival/pickup
    lcd.clear();
    lcd.print("Please take item");
    bool picked = waitForPickup();

    lcd.clear();
    if (picked) {
      lcd.print("Delivery complete");
      tone(BUZZER, 1200, 300); // success beep
    } else {
      lcd.print("No pickup timeout");
      tone(BUZZER, 400, 500); // fail beep
    }
    delay(2000);

    //spray
    sprayRoutine();

    //Reset to press button
    lcd.clear();
    lcd.print("Press button");
  }
}
