/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the ID/UID, type and any data blocks it can read. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * If your reader supports it, this sketch/program will read all the PICCs presented (that is: multiple tag reading).
 * So if you stack two or more PICCs on top of each other and present them to the reader, it will first output all
 * details of the first and then the next PICC. Note that this may take some time as all data blocks are dumped, so
 * keep the PICCs at reading distance until complete.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

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

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
	Serial.begin(9600);		// Initialize serial communications with the PC
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  clock.begin();
  lcd.begin(16, 2);
  lcd.print("Please Scan");
}

void loop() {
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

	check(mfrc522.uid.uidByte[0], mfrc522.uid.uidByte[1], mfrc522.uid.uidByte[2], mfrc522.uid.uidByte[3]);
  delay(5000);
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

