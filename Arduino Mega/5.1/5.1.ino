/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/

// the setup function runs once when you press reset or power the board
unsigned long start;
unsigned long previousMillis = 0;
int frequency = 250;
int period = 500 / frequency; // using 500 to ms

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(2, OUTPUT);
  Serial.begin(9600); 
  start = millis(); // finding a start time
}

// the loop function runs over and over again forever
void loop() {
  unsigned long currentTime = millis(); // current time
  if (currentTime - start < 5000) { // Speaker turns off after 5000ms (comparing the current time to start time)
    digitalWrite(2, LOW);
    delay(period);
    digitalWrite(2, HIGH);
    delay(period);
  }
  if (currentTime - previousMillis > 200) { // checking once every 200ms to switch the lights
      previousMillis = currentTime;
      if (digitalRead(10) == HIGH) { // checking if light is on/off
        digitalWrite(10, LOW);
        digitalWrite(LED_BUILTIN, HIGH);
     } else {
        digitalWrite(10, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
  
}


