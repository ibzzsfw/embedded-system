#define PIN_LED PB1
#define PIN_SENSOR PB2
#define THRESHOLD 200

void setup() {
  // set PB1 as output
  DDRB |= (1 << PIN_LED);
  // set internal pull up resistor for PB2 (input)
  DDRB &= ~(1 << PIN_SENSOR);
  PORTB |= (1 << PIN_SENSOR);
  // set ADC to read from A1
  ADMUX |= (1 << MUX0); // MUX[3:0] = 0001
  // set voltage ref to AVcc
  ADMUX |= (1 << REFS0);
  // enable ADC
  ADCSRA |= (1 << ADEN);
  // set prescaler to 128 for sample rate
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void loop() {
  // start ADC conversion
  ADCSRA |= (1 << ADSC);
  // wait for conversion to complete
  while (ADCSRA & (1 << ADSC))
    ;

  uint64_t adcValue = ADC;
  if (adcValue > THRESHOLD)
    PORTB |= (1 << PIN_LED); // normally closed
  else
    PORTB &= ~(1 << PIN_LED); // normally open
  _delay_ms(1000);
}