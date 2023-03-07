#define COMMAND_PORT PORTD
#define COMMAND_DDR DDRD
#define RS (1 << PORTD2)
#define E (1 << PORTD4)

#define DATA_PORT PORTB
#define DATA_DDR DDRB

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

void init_ADC() {
  ADMUX |= (1 << REFS0);
  ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void setup() {
  init_LCD();
  init_ADC();
  send_string("Temperature: ");
}

char buffer[16];

void loop() {
  send_command(0x01);
  send_command(0x80);

  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC)) {
  }

  uint16_t adcValue = ADC;
  uint16_t temp = ((adcValue / 1024.0 * 5000.0) - 500.0) / 10.0;

  itoa(temp, buffer, 10);
  send_string(buffer);
  send_string(" Celcius");
  _delay_ms(1000);
}