// define port
// PB5 as a button input
#define BUTTON PINB5
// PB4 as a LED output
#define LED PORTB4

void waitForButtonPress() {
  // Wait for button to be pressed
  while (PINB & (1 << BUTTON))
    ;
  // Debounce
  _delay_ms(50);
}

void toggleLED() {
  // Toggle LED
  PORTB ^= (1 << LED);
}

void setup() {
  // Enable internal pull-up resistor
  PORTB |= (1 << PORTB5);
  // Set PB4 as output
  DDRB |= (1 << DDB4);
  // Set PB5 as input
  DDRB &= !(1 << DDB5);
}

void loop() {

  waitForButtonPress();
  toggleLED();
}