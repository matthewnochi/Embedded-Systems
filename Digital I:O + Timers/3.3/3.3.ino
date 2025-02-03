#define c 3830 // 261 Hz
#define d 3400 // 294 Hz
#define e 3038 // 329 Hz
#define f 2864 // 349 Hz
#define g 2550 // 392 Hz
#define a 2272 // 440 Hz
#define b 2028 // 493 Hz
#define C 1912 // 523 Hz
#define R 0


int light = 0;
int note = 0;
int notes[] = { e, R, d, R, c, R, d, R, e, R,e, R,e, R,d, R,d, R,d, R,e, R,g, R,g, R,e, R,d, R,c, R,d, R,e, R,e, R,e, R,e, R,d, R,d, R,e, R,d, R,c, R,c };
unsigned long prevNote = 0;
unsigned long prevLight = 0;
boolean lightOn = false;
boolean allLight = false;

void setup() {
  // task A set up
  DDRL |= 7 << DDL0;

  // task B set up
  DDRH |= (1 << PH3);

  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= 1 << WGM42; // CTC Mode
  TCCR4A |= 1 << COM4A0;  

  TCCR4B |= 2 << CS40; // sets prescalar
}

void loop() {
  taskC();
}

void taskA() {
  if (allLight) {
    PORTL &= ~(7 << PORTL0);
  }
  if (millis() - prevLight >= 333) {
    if (!lightOn) {
      if (light == 0) {
        PORTL |= 1 << PORTL0;
      } else if (light == 1) {
        PORTL |= 1 << PORTL1;
      } else {
        PORTL |= 1 << PORTL2;
      }
      lightOn = true;
      prevLight = millis();
    } else {
      PORTL &= ~(7 << PORTL0);
      light = (light + 1) % 3;
      lightOn = false;
    }
  }
}

void taskB() {
  if (note < sizeof(notes) / sizeof(notes[0])) {
    if (millis() - prevNote >= 400) {
      if (notes[note] == 0) {
        TCCR4A &= ~(1 << COM4A0);
      } else {
        TCCR4A |= 1 << COM4A0;
        OCR4A = notes[note];
      }
      note++;
      prevNote = millis();
    }
  } else {
    TCCR4A &= ~(1 << COM4A0);
  }
}

void taskC() {
  unsigned long loopStart = millis();
  while(millis() - loopStart <= 2000) {
    taskA();
  }
  allLight = true;
  taskA();
  allLight = false;
  loopStart = millis();
  while(millis() - loopStart <= (sizeof(notes) / sizeof(notes[0])) * 402) {
    taskB();
  }
  note = 0;

  loopStart = millis();
  while(millis() - loopStart <= 10000) {
    taskB();
    taskA();
    note = note % (sizeof(notes) / sizeof(notes[0]));
  }
  allLight = true;
  taskA();
  allLight = false;
  note = 999;
  taskB();
  note = 0;
  delay(1000);
}
