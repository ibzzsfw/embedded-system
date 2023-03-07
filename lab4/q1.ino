#define BAUD 9600
#define MYBURR (F_CPU / (16 * BAUD)) - 1

#define COMMAND_PORT PORTD
#define COMMAND_DDR DDRD
#define RS (1 << PORTD2)
#define E (1 << PORTD4)

#define DATA_PORT PORTB
#define DATA_DDR DDRB

#define TRUE 1

void pulse_E() {
  COMMAND_PORT |= E;
  _delay_us(10);
  COMMAND_PORT &= ~E;
  _delay_us(10);
}

void USART_Init(unsigned int ubrr) {
  /* Set BAUD rate */
  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)ubrr;
  /* Enable receiver and transmitter */
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  /* Set frame format: 8data, 2stop bit */
  UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

unsigned char USART_Receive() {
  /* Wait for data to be received */
  while (!(UCSR0A & (1 << RXC0)))
    ;
  /* Get and return received data from buffer */
  return UDR0;
}

void USART_Transmit(unsigned char data) {
  /* Wait for empty transmit buffer */
  while (!(UCSR0A & (1 << UDRE0)))
    ;
  /* Put data into buffer, sends the data */
  UDR0 = data;
}

void transmit(char *str) {
  while (*str) {
    USART_Transmit(*str);
    str++;
  }
}

void setup() { USART_Init(MYBURR); }

void loop() {
  char name[100];
  char greeting[] = "Hello ";
  int i = 0;

  while (TRUE) {
    name[i] = USART_Receive();
    if (name[i] == '.') {
      name[i] = '\0';
      break;
    }
    i++;
  }

  transmit(greeting);
  transmit(name);
  USART_Transmit('\n');
  memset(name, 0, sizeof(name));
}