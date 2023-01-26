void setup() {
  DDRB &= !(1 << DDB5);
  DDRB |= (1 << DDB4);
  PORTB |= (1 << PORTB5);
}

void loop() {
  while (PINB & (1 << PINB5))
    ;
  _delay_ms(50);
  PORTB ^= (1 << PORTB4);

  while (PINB & (1 << PINB5))
    ;
  _delay_ms(50);

  PORTB ^= (1 << PORTB4);
}
