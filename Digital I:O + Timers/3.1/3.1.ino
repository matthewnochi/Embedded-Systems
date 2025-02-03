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
}

void loop() {
  taskC();
}

void taskA() {
  PORTL |= 1 << PORTL0;
  delay(333);
  PORTL &= ~(1 << PORTL0);
  PORTL |= 1 << PORTL1;
  delay(333);
  PORTL &= ~(1 << PORTL1);
  PORTL |= 1 << PORTL2;
  delay(333);
  PORTL &= ~(1 << PORTL2);
}

void taskB() {
  TCCR4A |= 1 << COM4A0;
  OCR4A = 250000 / 400;
  delay(1000);
  OCR4A = 250000 / 250;
  delay(1000);
  OCR4A = 250000 / 800;
  delay(1000);
  TCCR4A &= ~(1 << COM4A0);
  delay(1000);
}

void taskC() {
  unsigned long loopStart = millis();
  while(millis() - loopStart < 2000) {
    taskA();
  }
  taskB();
  delay(1000);
}
