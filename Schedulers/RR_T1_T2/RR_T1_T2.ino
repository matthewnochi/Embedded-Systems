#define E 659 // Hz
#define C 523 // Hz
#define G 784 // Hz
#define g 392 // Hz
#define R 0 // Rest, play no sound
int song[] = {E, R, E, R, R, E, R, R, C, R, E, R, R, G, R, R, R, R, R, g, R};
int note = 0;
boolean light = false;

unsigned long lightTime = 0;
unsigned long noteTime = 0;
unsigned long noteWait = 100;

void setup() {
  // Task 1 set up
  DDRL |= 1 << DDL0; // set pin 49
  PORTL &= ~(1 << PORTL0); // initial off

  // Task 2 set up
  DDRH |= (1 << PH3); // set pin 6

  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= 1 << WGM42; // CTC Mode
  TCCR4A |= 1 << COM4A0; // toggle pin 6
  TCCR4B |= 1 << CS41; // prescaler 8

}

void loop() {
  while(1) {
    Task1();
    Task2();
    delay(1);
  }
}

void Task1() {
  if (light && (millis() - lightTime >= 250)) {
    PORTL &= ~(1 << PORTL0); // Turn off the LED
    light = false;
    lightTime = millis();
  } else if (millis() - lightTime >= 750) {
    PORTL |= 1 << PORTL0; // Turn on the LED
    light = true;
    lightTime = millis();
  }
}

void Task2() {
  if (millis() - noteTime >= noteWait) {
    if (note < sizeof(song) / sizeof(song[0])) {
      if (song[note] == 0) {
        TCCR4A &= ~(1 << COM4A0);
      } else {
        TCCR4A |= 1 << COM4A0;
        OCR4A = 2000000 / (2 * song[note]);
      }
      noteTime = millis();
      noteWait = 100;
    }
    if (note >= sizeof(song) / sizeof(song[0])) {
      TCCR4A &= ~(1 << COM4A0);
      note = 0;
      noteTime = millis();
      noteWait = 4000;
    } else {
      note++;
    }
  }
}


