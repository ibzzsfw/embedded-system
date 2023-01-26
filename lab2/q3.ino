void setup() {
  // ser pin3 as output
  DDRB |= (1 << DDB3);
  // set pin4 and pin5 as input
  DDRB &= !(1 << DDB4) & !(1 << DDB5);
  // internal pullup for pin4 and pin5
  PORTB |= (1 << PORTB4) | (1 << PORTB5);
}

void loop() {
  // if pin4 is high or pin5 is high
  if (PINB & (1 << PINB4) || PINB & (1 << PINB5)) {
    // set pin3 high
    PORTB |= (1 << PORTB3);
  } else {
    // set pin3 low
    PORTB &= !(1 << PORTB3);
  }
}
