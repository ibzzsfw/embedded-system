#define F_CPU 8000000L

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

#define DELAY_MS 1000L
#define HOLD 3000L

/* Program for AVR;
- 3 leds loop to blink every DELAY_MS ms
- immediately press button to stop looping (only one of them turn on)
- long press (HOLD ms) to turn of all leds

- LED1 : PB1
- LED2 : PC2
- LED3 : PC1

*/

volatile bool looping = true;
volatile bool off = false;
int hold_time = 0;

void turn_off_all() {
  PORTB &= ~_BV(PB1);
  PORTC &= ~(_BV(PC1) | _BV(PC2));
}

void turn_on_case(int led) {
  turn_off_all();

  if (led == 0) {
    PORTB |= _BV(PB1);
  }
  if (led == 1) {
    PORTC |= _BV(PC1);
  }
  if (led == 2) {
    PORTC |= _BV(PC2);
  }
}

void set_TCNT1() { TCNT1 = 0xFFFF - (DELAY_MS * (F_CPU / 1000L) / 1024L); }

void setup() {
  // set PB1, PC1, PC2 as output
  DDRB |= _BV(PB1);
  DDRC |= _BV(PC1) | _BV(PC2);

  // Set button pin as input with pull-up resistor enabled
  DDRD &= ~_BV(DDD2);
  PORTD |= _BV(PORTD2);

  // Enable external interrupt on INT0
  EIMSK |= _BV(INT0);

  // Trigger interrupt on falling edge
  EICRA |= _BV(ISC01);

  // this section for timer1 interrupt DELAY_MS in normal mode
  // Set timer to normal mode
  TCCR1A = 0;
  TCCR1B |= _BV(CS10) | _BV(CS11);

  // Enable timer interrupt
  TIMSK1 |= _BV(TOIE1);

  // enable global interrupt
  sei();
}

ISR(INT0_vect) {
  looping = !looping;
  set_TCNT1();
  hold_time = 0; // reset hold_time on button press
}

ISR(TIMER1_OVF_vect) {
  // global state for led
  static int state = 0;

  // check if long press
  if (hold_time >
      HOLD / DELAY_MS) { // check if hold_time is greater than HOLD/DELAY_MS
    turn_off_all();
    off = true;
    return;
  }

  // check if looping
  if (looping) {
    turn_on_case(state);
    state = (state + 1) % 3;
  }

  // check if button is pressed
  hold_time += !(PIND & _BV(PIND2));

  set_TCNT1();
}

int main(void) {
  setup();

  while (true)
    ;

  return (0);
}