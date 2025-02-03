void setup() {
  DDRH |= (1 << PH3);

  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= 1 << WGM42; // CTC Mode
  TCCR4A |= 1 << COM4A0;  

  TCCR4B |= 3 << CS40; // sets prescalar to 64 (250000Hz)
}

void loop() {
  OCR4A = 250000 / 400;
  delay(1000);
  OCR4A = 250000 / 250;
  delay(1000);
  OCR4A = 250000 / 800;
  delay(1000);
  TCCR4A &= ~(1 << COM4A0);
  delay(1000);
  TCCR4A |= 1 << COM4A0;
}
