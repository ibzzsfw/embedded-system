/*
Schema:

- command -
RS - DDD2
E - DDD4

- data -
D4...7 - DDB8...11
*/

void pulseE() {
  PORTD |= (1 << PORTD4);
  _delay_us(10);
  PORTD &= ~(1 << PORTD4);
  _delay_us(10);
}

void sendCommand(uint8_t command) {
  PORTD &= ~(1 << PORTD2); // set RS to 0 (command)
  PORTB = (PORTB & 0xF0) | (command >> 4);
  pulseE();
  PORTB = (PORTB & 0xF0) | (command & 0x0F);
  pulseE();
}

void sendData(uint8_t data) {
  PORTD |= (1 << PORTD2); // set RS to 1 (data)
  PORTB = (PORTB & 0xF0) | (data >> 4);
  pulseE();
  PORTB = (PORTB & 0xF0) | (data & 0x0F);
  pulseE();
}

void sendString(const char *str) {
  while (*str) {
    sendData(*str);
    str++;
  }
}

void initLCD() {
  const uint8_t lcd_init_cmds[] = {0x33, 0x32, 0x28, 0x0E, 0x01, 0x80};
  const int cmds_count = 6;

  // set data pins as output
  DDRB |= 0x0F;
  PORTB &= 0xF0;

  // set command pins as output
  DDRD |= (1 << DDD2) | (1 << DDD4);        // RS, E respectively
  PORTD &= ~(1 << PORTD2) & ~(1 << PORTD4); // set RS, E to 0

  for (int i = 0; i < cmds_count; i++) {
    sendCommand(lcd_init_cmds[i]);
  }
}

void setup() {
  initLCD();
  sendString("Hello World!");
}

void loop() {}