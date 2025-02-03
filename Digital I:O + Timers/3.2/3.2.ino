int light = 0;
int note = 0;
int notes[] = {625, 1000, 312, 0};
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

  TCCR4B |= 3 << CS40; // sets prescalar to 64 (250000Hz)

  // setup for task C
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
    if (millis() - prevNote >= 1000) {
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
  while(millis() - loopStart <= 4000) {
    taskB();
  }
  note = 0;

  loopStart = millis();
  while(millis() - loopStart <= 10000) {
    taskB();
    taskA();
    note = note % 4;
  }
  allLight = true;
  taskA();
  allLight = false;
  note = 99;
  taskB();
  note = 0;
  delay(1000);
}
