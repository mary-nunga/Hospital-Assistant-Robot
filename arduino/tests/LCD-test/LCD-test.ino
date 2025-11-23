#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // address, columns, rows

void setup() {
  lcd.init();          // initialize the LCD
  lcd.backlight();     // turn on backlight

  lcd.setCursor(0, 0);
  lcd.print("MediBot Ready");

  lcd.setCursor(0, 1);
  lcd.print("LCD Working :)");
}

void loop() {
}
