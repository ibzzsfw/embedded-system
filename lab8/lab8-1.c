#define F_CPU 8000000L

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

/*
Lab 8.1 AVR Timer: PWN

Dimmer: Adjust the brightness of an LED using a potentiometer.
*/

// LED
#define LED_DDR DDRD
#define LED_PIN _BV(PD6)

volatile uint16_t adcValue = 0;

void initADC() {
  ADMUX |= _BV(REFS0); // Set reference voltage to AVCC

  ADCSRA |= _BV(ADEN)     // Enable ADC
            | _BV(ADATE)  // Enable auto-trigger
            | _BV(ADIE)   // Enable ADC interrupt
            | _BV(ADPS2)  // Set prescaler to 128
            | _BV(ADPS1)  // 128 = 125kHz
            | _BV(ADPS0); // 8MHz/128 = 62.5kHz

  ADCSRB = 0; // Free running mode
}

void initPWM() {
  TCCR0A = _BV(WGM01)     // Fast PWM mode
           | _BV(WGM00)   // Fast PWM mode
           | _BV(COM0A1); // Clear OC0A on compare match, set OC0A at BOTTOM

  TCCR0B = _BV(CS02); // Prescaler = 256, F_PWM = F_CPU/(256*256)

  OCR0A = 0xFF; // Set PWM duty cycle
}

ISR(ADC_vect) { adcValue = ADC; }

void setup() {
  initADC();
  initPWM();
  LED_DDR |= LED_PIN; // Set LED pin as output
  _delay_ms(1000);
  sei();
  ADCSRA |= _BV(ADSC); // Start ADC conversion
}

void loop() {
  OCR0A = (adcValue >> 2); // Set PWM duty cycle (divide by 4)
}

int main(void) {
  setup();
  while (1) {
    loop();
  }
}