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

void send_command(uint8_t command) {
  COMMAND_PORT &= ~RS;
  DATA_PORT = (DATA_PORT & 0xF0) | (command >> 4);
  pulse_E();
  DATA_PORT = (DATA_PORT & 0xF0) | (command & 0x0F);
  pulse_E();
}

void send_data(uint8_t data) {
  COMMAND_PORT |= RS;
  DATA_PORT = (DATA_PORT & 0xF0) | (data >> 4);
  pulse_E();
  DATA_PORT = (DATA_PORT & 0xF0) | (data & 0x0F);
  pulse_E();
}

void send_string(const char *str) {
  while (*str) {
    send_data(*str);
    str++;
  }
}

void init_LCD() {

  const uint8_t lcd_init_cmds[] = {0x33, 0x32, 0x28, 0x0E, 0x01, 0x80};
  const int cmds_count = 6;

  DDRB |= 0x0f;
  PORTB &= 0xF0;

  DDRD |= (1 << DDD2) | (1 << DDD4);
  PORTD &= ~(1 << PORTD2) & ~(1 << PORTD4);

  for (int i = 0; i < cmds_count; i++) {
    send_command(lcd_init_cmds[i]);
  }
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

void setup() {
  USART_Init(MYBURR);
  init_LCD();
}

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

  send_command(0x01);
  send_string(greeting);
  send_string(name);
  memset(name, 0, sizeof(name));
}