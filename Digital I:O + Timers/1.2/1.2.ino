void setup() {
  DDRL |= (7 << DDL0);
}

void loop() {
  PORTL &= ~(1 << PORTL2);
  PORTL |= (1 << PORTL0);
  delay(333);
  PORTL &= ~(1 << PORTL0);
  PORTL |= (1 << PORTL1);
  delay(333);
  PORTL &= ~(1 << PORTL1);
  PORTL |= (1 << PORTL2);
  delay(333);
}
