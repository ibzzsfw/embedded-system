#define RED_LED PB0
#define BLUE_LED PB1
#define GREEN_LED PB2

#define BLACK 0
#define WHITE 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define YELLOW 5
#define CYAN 6
#define MAGENTA 7

void setup(void) {
  DDRB = 0b00111111;
  PORTB = 0b00111000;
}

void loop(void) {
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
  setLED(color_lookup[(PINB & 0b00111000) >> 3]);
}

void setLED(bool *rgb) {
  PORTB &=
      ~(1 << RED_LED) & ~(1 << BLUE_LED) & ~(1 << GREEN_LED); // clear all LEDs
  PORTB |= (rgb[0] << RED_LED) | (rgb[1] << GREEN_LED) | (rgb[2] << BLUE_LED);
}
