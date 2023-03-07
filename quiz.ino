// from this circuit, Write code to implement a traffic light as this
// instruction
/* 1) first RED led is on */
/* 2) then if push the button, wait for 5 seconds then turn off RED led and
turn
 * on the YELLOW led for 5 seconds, */
/* 3) then turn off YELLOW led, turn on GREEN led for 5 seconds. Last turn
off
 * GREEN led and back to turn on RED led. */
// ATTiny85

#include <avr/io.h>
#include <util/delay.h>

#define RED PB4
#define YELLOW PB3
#define GREEN PB5

#define BUTTON PB0

#define DELAY 1000

void init() {
  DDRB |= (1 << RED) | (1 << YELLOW) | (1 << GREEN);
  DDRB &= ~(1 << BUTTON);
  PORTB |= (1 << BUTTON);
}
void turn_off_all() { PORTB &= ~((1 << RED) | (1 << YELLOW) | (1 << GREEN)); }

void turn_on_red() {
  turn_off_all();
  PORTB |= (1 << RED);
}

void turn_on_yellow() {
  turn_off_all();
  PORTB |= (1 << YELLOW);
}

void turn_on_green() {
  turn_off_all();
  PORTB |= (1 << GREEN);
}

void setup() {
  init();
  turn_on_red();
}

void loop() {
  if ((PINB & (1 << BUTTON)) == 0) {
    turn_on_yellow();
    _delay_ms(DELAY);
    turn_on_green();
    _delay_ms(DELAY);
  }
}