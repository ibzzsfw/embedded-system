/*
Keypad layout
1 2 3 A
4 5 6 B
7 8 9 C
* 0 # D

Keypad : port mapping
row1..4 : DDD7..4
col1..4 : DDD3..0

LED : port mapping
row1..4 : PORTC0..3
col1..4 : PORTB3..0
*/

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / 16 / BAUD) - 1)

#define DELAY_MS 20

const unsigned char keymap[4][4] = {{'1', '2', '3', 'A'},
                                    {'4', '5', '6', 'B'},
                                    {'7', '8', '9', 'C'},
                                    {'*', '0', '#', 'D'}};

void USART_Init(unsigned int ubrr) {
  /*
   * UBRR0H is the high byte of the UBRR0 register
   * UBRR0L is the low byte of the UBRR0 register
   * UCSR0B is the USART Control and Status Register B, which contains the RXEN0
   * and TXEN0 bits UCSR0C is the USART Control and Status Register C, which
   * contains the UCSZ00 and UCSZ01 bits RXEN0 is the Receive Enable bit TXEN0
   * is the Transmit Enable bit UCSZ00 is the Character Size bit 0 USBS0 is the
   * Stop Bit Select bit
   */

  /* Set BAUD rate */
  UBRR0H = (unsigned char)(ubrr >> 8); // >> 8 means shift 8 bits to the right
                                       // to get the first 8 bits
  UBRR0L = (unsigned char)ubrr;        // get the last 8 bits
  /* Enable receiver and transmitter */
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  /* Set frame format: 8data, 2stop bit */
  UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

unsigned char USART_Receive() {
  /*
   * UCSR0A is the USART Control and Status Register A, which contains the RXC0
   * bit RXC0 is the Receive Complete bit UDR0 is the USART I/O Data Register
   */

  /* Wait for data to be received */
  while (!(UCSR0A & (1 << RXC0)))
    ;
  /* Get and return received data from buffer */
  return UDR0;
}

void USART_Transmit(unsigned char data) {
  /*
   * UCSR0A is the USART Control and Status Register A, which contains the UDRE0
   * bit UDRE0 is the USART Data Register Empty bit UDR0 is the USART I/O Data
   * Register
   */

  /* Wait for empty transmit buffer */
  while (!(UCSR0A & (1 << UDRE0)))
    ;
  /* Put data into buffer, sends the data */
  UDR0 = data;
}

// Goal: press a key on the keypad and see the corresponding LED light up
void setup() {
  // initialize serial communication at 9600 bits per second:
  USART_Init(UBRR_VALUE);
  // keypad - col
  DDRD = 0x0F;
  // keypad - row
  PORTD = 0xF0;
  // led - col
  DDRB = 0x0F;
  // led - row
  DDRC = 0x0F;
}

unsigned char col = 0x01;
unsigned char row;

void loop() {

  // wait for button press
  while ((PIND & 0xF0) == 0xF0) {
    col = (col == 0x10) ? 0x01 : (col << 1); // shift left
    PORTD = 0xF0 | ~(col & 0x0F); // set col by active low
    _delay_ms(DELAY_MS); // delay
    row = (PIND & 0xF0); // get row
  }

  // set led for COL (active low)
  PORTB = 0xF0 | ~(col & 0x0F);
  // set led for ROW (active low)
  PORTC = 0xF0 | (row >> 4);
}
