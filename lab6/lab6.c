#define F_CPU 8000000L

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

const int DELAY = 2000;

volatile bool ON = true;

void setup() {
  // Set LED pin as output
  DDRB |= _BV(DDB1);

  // Set button pin as input with pull-up resistor enabled
  DDRD &= ~_BV(DDD2);
  PORTD |= _BV(PORTD2);

  // Enable external interrupt on INT0
  EIMSK |= _BV(INT0);

  // Trigger interrupt on falling edge
  EICRA |= _BV(ISC01);

  // Enable global interrupts
  sei();
}

void blink() {
  PORTB ^= _BV(PORTB1);
  _delay_ms(DELAY);
}

void turn_off_led() { PORTB &= ~_BV(PORTB1); }

ISR(INT0_vect) { ON = !ON; }

int main(void) {
  setup();

  while (true) {
    if (ON) {
      blink();
    } else {
      turn_off_led();
    }
  }

  return (0);
}