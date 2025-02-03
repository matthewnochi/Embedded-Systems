#include <Arduino_FreeRTOS.h>

void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
void Task4Mario(void *pvParameters);
void Task3Blink(void *pvParameters);

void setup() {

  Serial.begin(19200);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  } 

  xTaskCreate(
    TaskBlink
    ,  "Blink"   // name 
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskAnalogRead
    ,  "AnalogRead"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

    xTaskCreate(
    Task3Blink
    ,  "OffBoard-Flash"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

    xTaskCreate(
    Task4Mario
    ,  "MarioSong"
    ,  256  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );

  Serial.println("Setup Complete");
  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}

void TaskBlink(void *pvParameters)  // This is a task.
{

  // initialize digital LED_BUILTIN on pin 13 as an output.
  DDRB |= (1 << PB7);

  for (;;) // A Task shall never return or exit.
  {
    PORTB |= 1 << PORTB7;   // LED on
    vTaskDelay( 250 / portTICK_PERIOD_MS ); // wait for 0.250 seconds
    PORTB &= ~(1 << PORTB7);   // LED off 
    vTaskDelay( 100 / portTICK_PERIOD_MS ); // wait for 0.100 seconds
  }
}

void TaskAnalogRead(void *pvParameters)  // This is a task.
{
  for (;;)
  {
    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);  /// modify for your input pin!
    // print out the value you read:
    Serial.println(sensorValue);
    vTaskDelay(500/portTICK_PERIOD_MS);  // 0.5 sec in between reads for stability
  }
}

void Task3Blink(void *pvParameters)  // This is a task.
{

  // initialize pin 12 as an output.
  DDRB |= (1 << PB6);
  PORTB &= ~(1 << PORTB6);

  for (;;) // A Task shall never return or exit.
  {
    PORTB |= 1 << PORTB6;
    vTaskDelay( 100 / portTICK_PERIOD_MS); // wait for 100ms 
    PORTB &= ~(1 << PORTB6);    // LED off
    vTaskDelay( 200 / portTICK_PERIOD_MS); // wait for 200ms
  }
}

void Task4Mario(void *pvParameters)  // This is a task.
{

  DDRH |= (1 << PH3); // set pin 6
  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= 1 << WGM42; // CTC Mode
  TCCR4A |= 1 << COM4A0; // toggle pin 6
  TCCR4B |= 1 << CS41; // prescaler 8

  // Frequencies for Mario Song 
  // Calculated by 16MHz / (prescalar * 2 * Frequency)
  int E = 1517; // 659Hz
  int C = 1912; // 523Hz
  int G = 1275; // 784Hz
  int g = 2551; // 392Hz
  int R = 0; // Rest, play no sound

  int song[] = {E, R, E, R, R, E, R, R, C, R, E, R, R, G, R, R, R, R, R, g, R};
  int note = 0;
  int playCount = 0;
  for (;;) // A Task shall never return or exit.
  {
    if (playCount >= 3) {
      vTaskSuspend(NULL);
    }
    if (note < sizeof(song) / sizeof(song[0])) {
      if (song[note] == 0) {
        TCCR4A &= ~(1 << COM4A0);
      } else {
        TCCR4A |= 1 << COM4A0;
        OCR4A = song[note];
      }
      vTaskDelay(100/ portTICK_PERIOD_MS);
      note++;
    } else {
      vTaskDelay(1500/ portTICK_PERIOD_MS);
      note = 0;
      playCount++;
    }
  }
}