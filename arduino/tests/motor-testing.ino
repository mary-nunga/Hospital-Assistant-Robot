#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// STATE
bool startDelivery = false;
String uartBuffer = "";

void sendStatus(const char *msg) {
  Serial.print("STATUS:");
  Serial.println(msg);
}

void showReady() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MediBot READY");
  lcd.setCursor(0, 1);
  lcd.print("Waiting command");
}

void processCommand(String cmd) {
  cmd.trim();

  if (cmd == "START_DELIVERY") {
    if (!startDelivery) {
      startDelivery = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Delivery started");
      sendStatus("DELIVERY_STARTED");
    }
  } else if (cmd == "EMERGENCY_STOP") {
    // later: stop motors, pump, etc.
    startDelivery = false;
    showReady();
    sendStatus("EMERGENCY_STOPPED");
  }
}

void checkUART() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      processCommand(uartBuffer);
      uartBuffer = "";
    } else {
      uartBuffer += c;
    }
  }
}

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  showReady();
  sendStatus("READY");
}

void loop() {
  checkUART();

  if (!startDelivery) {
    // idle: just keep listening for commands
    return;
  }

  // here is where we'll later plug:
  // - obstacle check
  // - IR tray logic
  // - spray phase
  // - and send more STATUS:... messages

}
