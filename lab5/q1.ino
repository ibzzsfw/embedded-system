const unsigned char ROW_PIN_MASK = 0xF0;
const unsigned char COL_PIN_MASK = 0x0F;
const unsigned char DELAY_MS = 20;

void setup() {
  // set PORTD0-3 as output of keypad (COL 1-4)
  DDRD = COL_PIN_MASK;
  /*
        set internal pull up resistor at PORTD4-7
        for input of keypad (ROW 1-4)
  */
  PORTD |= ROW_PIN_MASK;
  // set PORTB0-3 as output (COL)
  DDRB |= COL_PIN_MASK;
  // clear PORTB0-3
  PORTB &= ROW_PIN_MASK;
  // set PORTC0-3 as output (ROW)
  DDRC |= COL_PIN_MASK;
  // clear PORTC0-3
  PORTC &= ROW_PIN_MASK;
}

unsigned char col = 0x01;
unsigned char row;
void loop() {
  /*
        mask with 1111|XXXX
        wait for button press (active low)
  */
  while ((PIND & ROW_PIN_MASK) == ROW_PIN_MASK) {

    col = (col == 0x10) ? 0x01 : (col << 1);
    // set present column = 0
    PORTD = ROW_PIN_MASK | ~(col & COL_PIN_MASK);
    _delay_ms(DELAY_MS);
    // set present row
    row = (PIND & ROW_PIN_MASK);
  }
  // set led for COL (active low)
  PORTB = ROW_PIN_MASK | ~(col & COL_PIN_MASK);
  // set led for ROW (active low)
  PORTC = ROW_PIN_MASK | (row >> 4);
}