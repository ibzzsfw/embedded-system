#define F_CPU 8000000L

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

#define DELAY_MS 2000L

volatile bool ON = true;

void turn_on_led() { PORTB |= _BV(PORTB1); }
void turn_off_led() { PORTB &= ~_BV(PORTB1); }
void toggle_led() { PORTB ^= _BV(PORTB1); }

void set_TCNT1() { TCNT1 = 0xFFFF - (DELAY_MS * (F_CPU / 1000L) / 1024L); }

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

  // this section for timer interrupt DELAY_MS in normal mode
  // Set timer to normal mode
  TCCR1A = 0;
  TCCR1B |= _BV(CS10) | _BV(CS11);

  // Enable timer interrupt
  TIMSK1 |= _BV(TOIE1);

  set_TCNT1();

  // Enable interrupts
  sei();
}

ISR(INT0_vect) {
  turn_off_led();
  ON = !ON;
  set_TCNT1();
}

ISR(TIMER1_OVF_vect) {
  if (ON) {
    toggle_led();
  }
  set_TCNT1();
}

int main(void) {
  setup();
  while (true)
    ;
  return (0);
}