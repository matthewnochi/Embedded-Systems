#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal.h>

const int rs = 11, en = 8, d4 = 6, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

DS3231 clock;
RTCDateTime dt;

bool clocked[10] = {false};
byte id[10][4];
int times[10][2];

void setup() {
	Serial.begin(19200);
	while (!Serial);
  clock.begin();
  lcd.begin(16, 2);
  lcd.print("Please Scan");
}

void loop() {
  delay(3000);
  check(0x00, 0x00, 0x00, 0x00); 
  delay(6000);
  check(0x11, 0x11, 0x11, 0x11);
  delay(6000);
  check(0x00, 0x00, 0x00, 0x11);
  delay(6000);
}

void check(byte A, byte B, byte C, byte D) {
  int index = -1;
  bool out = false;
  
  for (int i = 0; i < 10; i++) {
    if (A == id[i][0] && B == id[i][1] && C == id[i][2] && D == id[i][3]) {
      index = i;
      out = clocked[i]; 
      break;
    }

    if (index == -1 && !clocked[i]) {
      index = i;
    }
  }

  dt = clock.getDateTime();

  if (out) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Clock In " + formatTime(times[index][0], times[index][1]));
    lcd.setCursor(0, 1);
    lcd.print("Clock Out " + formatTime(dt.hour, dt.minute));
    clocked[index] = false; 
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Clock In " + formatTime(dt.hour, dt.minute));

    clocked[index] = true;
    times[index][0] = dt.hour;
    times[index][1] = dt.minute;
    id[index][0] = A;
    id[index][1] = B;
    id[index][2] = C;
    id[index][3] = D;
  }

  delay(4000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Please Scan");
}


String formatTime(int hour, int minute) {
  String timeStr = "";
  if (hour < 10) timeStr += "0";
  timeStr += String(hour) + ":";
  if (minute < 10) timeStr += "0";
  timeStr += String(minute);
  return timeStr;
}
