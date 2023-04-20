#define F_CPU 8000000L

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

/*

AVR program for coundown circuit, decrese counter by 1 every 1 second
1. Push button 1: single press to add startin time for 5 seconds
2. Start countdown when by pressing button 2. During countdown, single press
button 2 to stop countdown
3. During countdown, single press button 1 to restart countdown
4. If we want to reset countdown time, press button 1 for 3 seconds
5. Display time every step of countdown

*/

// LCD
#define COMMAND_PORT PORTB
#define COMMAND_DDR DDRB
#define DATA_PORT PORTC
#define DATA_DDR DDRC

#define RS _BV(PB1)
#define E _BV(PB2)

#define DB4 _BV(PC0)
#define DB5 _BV(PC1)
#define DB6 _BV(PC2)
#define DB7 _BV(PC3)

#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02
#define SET_CURSOR 0x80

// Push button 1
#define BUTTON1_PORT PORTD
#define BUTTON1_DDR DDRD
#define BUTTON1_PIN PIND
#define BUTTON1 _BV(PD2)

// Push button 2
#define BUTTON2_PORT PORTD
#define BUTTON2_DDR DDRD
#define BUTTON2_PIN PIND
#define BUTTON2 _BV(PD3)

#define STEP_INCREASE_TIME 5 // 5 seconds
#define HOLD_BUTTON_TIME 3   // 3 seconds
#define DELAY_TIME 1000      // 1 second

#define TCNT1_MAX 65535
#define PRESCALER 1024

int start_time = 0;
int current_time = 0;
int hold_time = 0;
int is_countdown = 0;
int stop = 0;

void pulseE() {
  COMMAND_PORT |= E;
  _delay_ms(2);
  COMMAND_PORT &= ~E;
}

void sendCommand(uint8_t command) {
  COMMAND_PORT &= ~RS;
  DATA_PORT = (DATA_PORT & 0xF0) | (command >> 4);
  pulseE();
  DATA_PORT = (DATA_PORT & 0xF0) | (command & 0x0F);
  pulseE();
}

void sendData(uint8_t data) {
  COMMAND_PORT |= RS;
  DATA_PORT = (DATA_PORT & 0xF0) | (data >> 4);
  pulseE();
  DATA_PORT = (DATA_PORT & 0xF0) | (data & 0x0F);
  pulseE();
}

void sendString(const char *str) {
  while (*str) {
    sendData(*str);
    str++;
  }
}

void initLCD() {

  const int cmds_count = 6;
  const uint8_t lcd_init_cmds[] = {
      0x33, // 8-bit mode
      0x32, // 8-bit mode
      0x28, // 2 lines, 5x8 font
      0x0E, // Display on, cursor on, blink on
      0x01, // Clear display
      0x80  // Set cursor to 0,0
  };

  // Set up the command port
  COMMAND_DDR |= (RS | E);

  // Set up the data port
  DATA_DDR |= (DB4 | DB5 | DB6 | DB7);

  for (int i = 0; i < cmds_count; i++) {
    sendCommand(lcd_init_cmds[i]);
  }
}

void set_TCNT1() {
  TCNT1 = (int)(TCNT1_MAX - (DELAY_TIME * (F_CPU / 1000) / PRESCALER)); // 57723
}

void initTimer() {
  // Set up timer 1
  TCCR1A = 0;
  TCCR1B |= _BV(CS12) | _BV(CS10); // Set prescaler to 1024
  set_TCNT1();
}

void initINT0() {
  // Set button pin as input with pull-up resistor enabled
  BUTTON1_DDR &= ~BUTTON1;
  BUTTON1_PORT |= BUTTON1;
  // Set up INT0
  EIMSK |= _BV(INT0);  // Enable INT0
  EICRA |= _BV(ISC01); // Trigger INT0 on falling edge
}

void initINT1() {
  // Set button pin as input with pull-up resistor enabled
  BUTTON2_DDR &= ~BUTTON2;
  BUTTON2_PORT |= BUTTON2;
  // Set up INT0
  EIMSK |= _BV(INT1);  // Enable INT1
  EICRA |= _BV(ISC11); // Trigger INT0 on falling edge
}

void displayCountdownTime() {
  char buffer[16];
  sprintf(buffer, "Time: %d", (current_time));
  sendCommand(CLEAR_DISPLAY);
  sendCommand(RETURN_HOME);
  sendCommand(SET_CURSOR);
  sendString(buffer);
}

void displayMessage(const char *message) {
  sendCommand(CLEAR_DISPLAY);
  sendCommand(RETURN_HOME);
  sendCommand(SET_CURSOR);
  sendString(message);
}

void increaseStartTime() {
  start_time += STEP_INCREASE_TIME;
  current_time = start_time;
}

void restart() {
  stop = 1;
  current_time = start_time;
}

void resetTimer() {
  start_time = 0;
  current_time = 0;
  is_countdown = 0;
  stop = 0;
}

void countingDown() {
  if (is_countdown == 1 && stop == 0 && current_time >= 0) {
    current_time--;
    displayCountdownTime();
    if (current_time < 0) { // finish countdown
      stop = 0;
      displayMessage("Press 1: restart");
    }
  }
}

ISR(TIMER1_OVF_vect) {
  set_TCNT1();
  countingDown();
  // check if button 1 is pressed
  hold_time += !(BUTTON1_PIN & BUTTON1);
  // if press button 1 for 3 seconds, reset to start time
  if (hold_time >= HOLD_BUTTON_TIME) {
    resetTimer();
    displayMessage("Reset to 0");
  }
}

// Button 1
ISR(INT0_vect) {
  hold_time = 0;
  if (is_countdown == 0) {
    increaseStartTime();
  } else {
    restart();
  }
  displayCountdownTime();
}

// Button 2
ISR(INT1_vect) {
  if (is_countdown == 1) {
    if (stop == 1) {
      stop = 0;             // resume countdown
      TIMSK1 |= _BV(TOIE1); // Enable overflow interrupt
    } else {
      stop = 1;              // stop countdown
      TIMSK1 &= ~_BV(TOIE1); // Disable overflow interrupt
    }
  } else {
    is_countdown = 1; // start countdown
    stop = 0;
    TIMSK1 |= _BV(TOIE1); // Enable overflow interrupt
  }
}

void setup() {

  // Set up the LCD
  initLCD();

  // Set up the timer
  initTimer();

  // Set up the interrupt
  initINT0();
  initINT1();

  displayCountdownTime();

  // Enable interrupts
  sei();
}

void loop() {}

int main() {
  setup();
  while (1) {
    loop();
  }
}
