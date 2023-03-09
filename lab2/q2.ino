// define led pins
#define LED_RED PB0
#define LED_BLUE PB1
#define LED_GREEN PB2

// define dip switch pins
#define DIP1 PB5
#define DIP2 PB4
#define DIP3 PB3

// define color lookup table
bool color_lookup[8][3] = {
    {0, 0, 0}, // black 0
    {1, 1, 1}, // white 1
    {1, 0, 0}, // red 2
    {0, 1, 0}, // green 3
    {0, 0, 1}, // blue 4
    {1, 1, 0}, // yellow 5
    {0, 1, 1}, // cyan 6
    {1, 0, 1}  // magenta 7
};

void setLED(bool *rgb) {
  // clear all LEDs
  PORTB &= ~(1 << LED_RED) & ~(1 << LED_BLUE) & ~(1 << LED_GREEN);
  // set LEDs
  PORTB |= (rgb[0] << LED_RED) | (rgb[1] << LED_GREEN) | (rgb[2] << LED_BLUE);
}

void setup() {
  // set led pins as output
  DDRB |= (1 << LED_RED) | (1 << LED_BLUE) | (1 << LED_GREEN);
  // set dip switch pins as input
  DDRB &= ~((1 << DIP1) | (1 << DIP2) | (1 << DIP3));
  // enable internal pull-up resistor
  PORTB |= (1 << DIP1) | (1 << DIP2) | (1 << DIP3);
}

void loop() {
  // read dip switch pins
  uint8_t dip_switch = (PINB & ((1 << DIP1) | (1 << DIP2) | (1 << DIP3))) >> 3;
  // set led color
  setLED(color_lookup[dip_switch]);
}