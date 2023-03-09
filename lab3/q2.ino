#define COMMAND_PORT PORTD
#define COMMAND_DDR DDRD
#define RS (1 << PORTD2)
#define E (1 << PORTD4)

#define DATA_PORT PORTB
#define DATA_DDR DDRB

#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02

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
  //  use ADC1 as an analog input
  ADMUX |= (1 << REFS0) | (1 << MUX0);
  //  enable ADC, set prescaler to 128
  ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void setup() {
  init_LCD();
  init_ADC();
  send_string("Temperature: ");
}

char buffer[16];

void loop() {
  send_command(CLEAR_DISPLAY);
  send_command(RETURN_HOME);

  ADCSRA |= (1 << ADSC);

  while (ADCSRA & (1 << ADSC))
    ;

  uint16_t adcValue = ADC;
  uint64_t Vout = (adcValue / 1024.0 * 5000.0);
  uint64_t RLDR = (50000000.0 - (Vout * 10000.0)) / Vout;
  uint64_t lightLevel = (1.25 * pow(10, 7)) * pow(RLDR, -1.4059);

  dtostrf(lightLevel, 4, 2, buffer);
  send_string(buffer);
  send_string(" lux");

  _delay_ms(1000);
}
