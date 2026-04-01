#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// ---------- SERVO ----------
Servo nozzleServo;
const int servoPin = 26;   // servo signal pin
const int minAngle = 60;   // left limit
const int maxAngle = 120;  // right limit
const int restAngle = 90;  // center

// ---------- ACEBOTT MOTOR CONTROL----------
// Direction bits 
const int M1_Forward  = 128; // 10000000  (left wheel forward)
const int M1_Backward = 64;  // 01000000  (left wheel backward)
const int M2_Forward  = 32;  // 00100000  (right wheel forward)
const int M2_Backward = 16;  // 00010000  (right wheel backward)

// Pin mapping for the Acebott car shield
const int SHCP_PIN = 18;   // shift clock
const int EN_PIN   = 16;   // enable
const int DATA_PIN = 5;    // data
const int STCP_PIN = 17;   // latch
const int PWM1_PIN = 19;   // PWM for motor speed

//  buzzer 
const int buzzerPin = 33;  

// ---------- LCD ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- Ultrasonic ----------
const int trigPin = 13;
const int echoPin = 14;
const int detectThreshold = 20;  // cm
const int clearThreshold  = 25;  // cm
bool obstacleDetected = false;

// ---------- IR Tray sensor ----------
const int irPin = 34;
int irThreshold = 180;  

// ---------- Relay ----------
const int relayPin = 27;

// ---------- STATE ----------
bool startDelivery = false;
bool waitingForTray = false;
bool trayEmptyEventSent = false;
String cmd;

//send status to web app
void sendStatus(const char *msg) {
  Serial.print("STATUS:");
  Serial.println(msg);
}

void sendDistance(long d) {
  Serial.print("DIST:");
  Serial.println(d);
}

void sendObstacle(bool o) {
  Serial.print("OBSTACLE:");
  Serial.println(o ? 1 : 0);
}

void sendTray(const char *t) {
  Serial.print("TRAY:");
  Serial.println(t);
}

void showReady() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MediBot READY");
  lcd.setCursor(0, 1);
  lcd.print("Waiting command");
}

// ---------- MOTOR CONTROL USING ACEBOTT DRIVER ----------

void Move(int Dir, int Speed)
{
  digitalWrite(EN_PIN, LOW);           //enable driver
  analogWrite(PWM1_PIN, abs(Speed));    // motor speed (0–255)

  digitalWrite(STCP_PIN, LOW);
  shiftOut(DATA_PIN, SHCP_PIN, MSBFIRST, Dir);
  digitalWrite(STCP_PIN, HIGH);
}

void moveForward() {
  Move(M1_Forward | M2_Forward, 255);
}

void moveBackward() {
  Move(M1_Backward | M2_Backward, 255);
}

void moveLeft() {
  // left wheel backward, right wheel forward
  Move(M1_Backward | M2_Forward, 255);
}

void moveRight() {
  // left wheel forward, right wheel backward
  Move(M1_Forward | M2_Backward, 255);
}

void stopMoving() {
  Move(0, 0); // all bits 0 -> stop
}

// ---------- SPRAY SEQUENCE ----------

void startSpraySequence() {
  sendStatus("SPRAYING");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sanitizing...");
  lcd.setCursor(0, 1);
  lcd.print("Please stand by");

  nozzleServo.write(minAngle);
  delay(150);

  digitalWrite(relayPin, HIGH);

  unsigned long start = millis();
  int angle = minAngle;
  bool right = true;

  while (millis() - start < 8000) {
    nozzleServo.write(angle);

    if (right) angle++;
    else       angle--;

    if (angle >= maxAngle) { angle = maxAngle; right = false; }
    if (angle <= minAngle) { angle = minAngle; right = true; }

    delay(10);
  }

  digitalWrite(relayPin, LOW);
  nozzleServo.write(restAngle);
  delay(150);

  sendStatus("COMPLETE");
  showReady();
  sendStatus("READY");

  startDelivery = false;
  waitingForTray = false;
  trayEmptyEventSent = false;
  obstacleDetected = false;
}

// ---------- COMMAND HANDLING ----------

void processCommand(String c) {
  c.trim();
  Serial.print("PROCESS: ");
  Serial.println(c);

  if (c == "START_DELIVERY") {
    Serial.println("STATUS:GOT_START");

    startDelivery = true;
    waitingForTray = false;
    trayEmptyEventSent = false;
    obstacleDetected = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Delivery started");
    sendStatus("DELIVERY_STARTED");
    sendTray("FULL");  // tray full at start

    moveForward();  
  }

  else if (c == "EMERGENCY_STOP") {
    startDelivery = false;
    waitingForTray = false;
    trayEmptyEventSent = false;
    obstacleDetected = false;

    digitalWrite(relayPin, LOW);
    stopMoving();
    noTone(buzzerPin);

    showReady();
    sendStatus("EMERGENCY_STOPPED");
  }

  else if (c == "AT_BLUE") {
    // robot "reached" destination – stop and wait for tray removal
    stopMoving();
    if (startDelivery && !trayEmptyEventSent) {
      waitingForTray = true;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("At dest point");
      lcd.setCursor(0, 1);
      lcd.print("Waiting pickup");

      sendStatus("WAITING_TRAY_EMPTY");
      sendTray("FULL");
    }
  }

  else if (c == "TRAY_FORCE_EMPTY") {
    // tray empty after timeout
    trayEmptyEventSent = true;
    waitingForTray = false;
    sendStatus("TRAY_EMPTY");
    sendTray("EMPTY");
  }

  else if (c == "AT_RED") {
    // reached sanitizing area – start spray
    if (startDelivery) {
      stopMoving();
      startSpraySequence();
    }
  }
}

// ---------- SENSORS ----------

long getDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 0;
  return duration * 0.034 / 2;
}

long getSmoothedDistanceCM() {
  long sum = 0;
  const int samples = 5;
  for (int i = 0; i < samples; i++) {
    sum += getDistanceCM();
    delay(5);
  }
  return sum / samples;
}

int readIR() {
  long sum = 0;
  const int samples = 10;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(irPin);
    delay(3);
  }
  return sum / samples;
}

// ---------- SERIAL READER ----------

void checkUART() {
  if (Serial.available()) {
    cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      Serial.print("CMD: ");
      Serial.println(cmd);
      processCommand(cmd);
    }
  }
}

// ---------- SETUP ----------

void setup() {
  Serial.begin(115200);
  delay(1000);  

  // Acebott motor driver pins
  pinMode(SHCP_PIN, OUTPUT);
  pinMode(EN_PIN,   OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(STCP_PIN, OUTPUT);
  pinMode(PWM1_PIN, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(irPin, INPUT);
  pinMode(relayPin, OUTPUT);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  stopMoving();
  digitalWrite(relayPin, LOW);

  lcd.init();
  lcd.backlight();

  nozzleServo.attach(servoPin);
  nozzleServo.write(restAngle);

  showReady();
  sendStatus("READY");
}

// ---------- MAIN LOOP ----------

void loop() {
  // always listen for commands
  checkUART();

  // continuously send distance & obstacle state
  long d = getSmoothedDistanceCM();
  sendDistance(d);

  // obstacle logic only when delivery active
  if (startDelivery) {
    if (!obstacleDetected) {
      if (d > 0 && d < detectThreshold) {
        obstacleDetected = true;
        tone(buzzerPin, 2000);
        sendStatus("OBSTACLE_WAITING");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Obstacle ahead");
        lcd.setCursor(0, 1);
        lcd.print("Waiting...");
        stopMoving();
      }
    } else {
      if (d == 0 || d > clearThreshold) {
        obstacleDetected = false;
        noTone(buzzerPin);
        sendStatus("OBSTACLE_CLEARED");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Path clear");
        lcd.setCursor(0, 1);
        lcd.print("Resuming...");
        moveForward();
      }
    }
  }

  sendObstacle(obstacleDetected);

  // tray detection when waiting 
  if (waitingForTray && !trayEmptyEventSent) {
    int ir = readIR();

    if (ir < irThreshold) {
      trayEmptyEventSent = true;
      waitingForTray = false;

      sendStatus("TRAY_EMPTY");
      sendTray("EMPTY");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Delivered");
      lcd.setCursor(0, 1);
      lcd.print("Tray empty");
    } else {
      sendTray("FULL");
    }
  }

  delay(50);  
}

