/* 
Pin Setups -

LCD: 
  VSS: GND
  VDD: 5V
  V0: 10k Potentiometer
  RS: 11
  RW: GND
  E: 8
  D4: 5
  D5: 4
  D6: 3
  D7: 2
  A: 220 Ohm Resistor to 5V
  K: GND

RFID-RC522:
  3.3V: 3.3V
  RST: 9
  GND: GND
  MISO: 50
  MOSI: 51
  SCK: 52
  SDA: 10

DS1307:
  GND: GND
  VCC: 5V
  SDA: 20
  SCL: 21

Off-Board LED
  LED: 220 Ohm Resistor to 12

Analog Reader:
  10K Potentiometer to A0
*/

#include <Arduino_FreeRTOS.h>
#include <semphr.h> 
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal.h>

// Task prototypes
void TaskBlink(void *pvParameters);
void TaskAnalogRead(void *pvParameters);
void Task3Blink(void *pvParameters);
void TaskRFIDClockInOut(void *pvParameters); 

// LCD and RTC setup
const int rs = 11, en = 8, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
DS3231 clock;
RTCDateTime dt;

// RC522 setup
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);

SemaphoreHandle_t xSerialSemaphore;

void setup() {
  Serial.begin(19200);
  while (!Serial) {
    ;  // wait for serial port to connect
  }

  if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  }

  // Create FreeRTOS tasks
  xTaskCreate(TaskBlink, "Blink", 128, NULL, 3, NULL);
  xTaskCreate(TaskAnalogRead, "AnalogRead", 128, NULL, 2, NULL);
  xTaskCreate(Task3Blink, "OffBoard-Flash", 128, NULL, 3, NULL);
  xTaskCreate(TaskRFIDClockInOut, "RFIDClockInOut", 256, NULL, 1, NULL); 

  Serial.println("Setup Complete");
  vTaskStartScheduler();
}

void loop() {
  // FreeRTOS scheduler manages tasks, no need for loop() code here.
}

void TaskBlink(void *pvParameters) {
  DDRB |= (1 << PB7);  // LED setup
  for (;;) {
    PORTB |= 1 << PORTB7;   // LED on
    vTaskDelay(250 / portTICK_PERIOD_MS);  // wait for 0.250 seconds
    PORTB &= ~(1 << PORTB7);   // LED off
    vTaskDelay(100 / portTICK_PERIOD_MS);  // wait for 0.100 seconds
  }
}

void TaskAnalogRead(void *pvParameters) {
  for (;;) {
    int sensorValue = analogRead(A0);  // Read analog input
    Serial.println(sensorValue);
    vTaskDelay(500 / portTICK_PERIOD_MS);  // Delay for stability
  }
}

void Task3Blink(void *pvParameters) {
  DDRB |= (1 << PB6);  // Pin 12 setup
  PORTB &= ~(1 << PB6);
  for (;;) {
    PORTB |= 1 << PORTB6;  // LED on
    vTaskDelay(100 / portTICK_PERIOD_MS);  // 100ms delay
    PORTB &= ~(1 << PB6);  // LED off
    vTaskDelay(200 / portTICK_PERIOD_MS);  // 200ms delay
  }
}

// New task for RFID reading and clocking in/out
void TaskRFIDClockInOut(void *pvParameters) {
  SPI.begin();
  mfrc522.PCD_Init();
  clock.begin();
  lcd.begin(16, 2);
  lcd.print("Please Scan");

  for (;;) {
    if (!mfrc522.PICC_IsNewCardPresent()) {
      vTaskDelay(100 / portTICK_PERIOD_MS);  // Delay to allow other tasks to run
      continue;
    }

    if (!mfrc522.PICC_ReadCardSerial()) {
      vTaskDelay(100 / portTICK_PERIOD_MS);  // Delay to allow other tasks to run
      continue;
    }

    // Call check function to handle the RFID UID and clock-in/out
    check(mfrc522.uid.uidByte[0], mfrc522.uid.uidByte[1], mfrc522.uid.uidByte[2], mfrc522.uid.uidByte[3]);
    vTaskDelay(4000 / portTICK_PERIOD_MS);
  }
}

// Check if the card is valid and clock it in or out
void check(byte A, byte B, byte C, byte D) {
  static bool clocked[10] = {false};
  static byte id[10][4];
  static int times[10][2];

  if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ) {
    int index = -1;
    bool out = false;

    // Find if the card is already in the system and whether it's clocked in or out
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

    dt = clock.getDateTime();  // Get current time

    // Handle clock-in/out logic
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
  }
  xSemaphoreGive( xSerialSemaphore );

  // Wait for 4 seconds before allowing a new scan
  vTaskDelay(4000 / portTICK_PERIOD_MS);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Please Scan");
}

// Format the time string for clocking
String formatTime(int hour, int minute) {
  String timeStr = "";
  if (hour < 10) timeStr += "0";
  timeStr += String(hour) + ":";
  if (minute < 10) timeStr += "0";
  timeStr += String(minute);
  return timeStr;
}
