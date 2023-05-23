/**
Project Name:	NUMBLE — 5-digit number guessing game

Member:
  Suppakorn Rakna	  63070501061
  Aussadawut Ardrit	63070501084

TinkerCAD	https://kmutt.me/cpe328-numble
*/

#define F_CPU 8000000L
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <util/delay.h>

// Game
#define GUESS_COUNT 5
#define DIGIT_COUNT 5
const char CORRECTED = 'O';
const char INCORRECTED = '-';
const char WRONG_POSITION = 'X';

// LCD
#define COMMAND_PORT PORTC
#define COMMAND_DDR DDRC

#define DATA_PORT PORTB
#define DATA_DDR DDRB

#define RS _BV(PC4)
#define E _BV(PC5)

#define DB4 _BV(PB0)
#define DB5 _BV(PB1)
#define DB6 _BV(PB2)
#define DB7 _BV(PB3)

#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02
#define BIGIN_SECOND_LINE 0xC0
#define BEGIN_FIRST_LINE 0x80
#define SECOND_ON_BLINK 0xCF
#define FIRST_OFF_OFF 0x8C
#define FIRST_SHIFT_RIGHT 0x86
#define SECOND_SHIFT_RIGHT 0xC6

// Keypad
#define ROW_PORT PORTD
#define ROW_DDR DDRD
#define COL_PORT PORTD
#define COL_DDR DDRD
#define COL_PIN PIND
#define ROW_PIN PIND

const unsigned char keypad[4][4] = {{'1', '2', '3', 'A'},
                                    {'4', '5', '6', 'B'},
                                    {'7', '8', '9', 'C'},
                                    {'*', '0', '#', 'D'}};

int is_guess = 0;
int is_correct = 0;
int guess_count = 0;
char answer[6];
char input[6];
int hints[6];
char header[17] = "-NUMBLE--NUMBLE-";
int i;
int j;
char digit;

int randomNumber = 0;

void pulseE() {
  COMMAND_PORT |= E;
  _delay_us(10);
  COMMAND_PORT &= ~E;
  _delay_us(10);
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

void initTimer() {
  // Set Timer1 prescaler to 64
  TCCR1B |= (1 << CS11) | (1 << CS10);

  // Enable Timer1 overflow interrupt
  TIMSK1 |= (1 << TOIE1);

  // Enable global interrupts
  sei();
}

char mapping(unsigned char x, unsigned char y) {
  unsigned char row = 0;
  unsigned char col = 0;
  for (unsigned char i = 0; i < 4; i++) {
    row += ((x >> i) & 0x01) * (3 - i);
    col += ((y >> i) & 0x01) * (3 - i);
  }
  return (keypad[row][col]);
}

char onKeyPress() {

  unsigned char col = 0x01;
  unsigned char row;

  while ((COL_PIN & 0xF0) == 0xF0) {
    col = (col == 0x10) ? 0x01 : (col << 1); // shift left
    COL_PORT = 0xF0 | ~(col & 0x0F);         // set col by active low
    _delay_ms(10);                           // delay
    row = (ROW_PIN & 0xF0);                  // get row
  }
  _delay_ms(200); // delay

  unsigned char x = ~(row >> 4) + '@';
  unsigned char y = col + '0';

  // get the digit from keypad layout
  return (mapping(x, y));
}

void backspace(int *i) {
  uint8_t command;

  --(*i);
  command = BEGIN_FIRST_LINE + (*i);
  sendCommand(command);
  sendData(' ');
  sendCommand(command);
}

void guess(int *i, int *cnt) {
  char digit;

  if (*cnt == 0) {
    sendCommand(CLEAR_DISPLAY);
  }
  sendCommand(SECOND_ON_BLINK);
  sendData(GUESS_COUNT - (*cnt) + '0');
  sendCommand(FIRST_OFF_OFF);
  sendString("life");
  sendCommand(RETURN_HOME);
  while (*i < DIGIT_COUNT) {
    digit = onKeyPress();
    if (digit >= '0' && digit <= '9') {
      input[(*i)++] = digit;
      sendData(digit);
    } else if (digit == 'D' && *i > 0) {
      backspace(i);
    }
  }
  input[*i] = '\0';
  (*cnt)++;
}

void feedback(char *messasge) {
  sendCommand(CLEAR_DISPLAY);
  sendCommand(RETURN_HOME);
  sendString(messasge);
  sendCommand(BIGIN_SECOND_LINE);
  sendString("# to restart");
}

char mainScreen() {
  char str[19];

  sendCommand(CLEAR_DISPLAY);
  sendCommand(RETURN_HOME);

  if (!guess_count) {
    sendString(header);
    sendCommand(BIGIN_SECOND_LINE);
    sendString("Press # to start");
  } else if (!is_correct) {
    sprintf(str, "Game over %d", randomNumber);
    feedback(str);
  } else {
    sprintf(str, "Correct (%d)", randomNumber);
    feedback(str);
  }

  return (onKeyPress());
}

void hint() {
  sendCommand(CLEAR_DISPLAY);
  sendCommand(FIRST_SHIFT_RIGHT);
  sendString(input);
  sendCommand(SECOND_SHIFT_RIGHT);

  // count number of each character and store in hint[i]
  for (int i = 0; i < 5; i++) {
    hints[i] = 0;
    for (int j = 0; j < DIGIT_COUNT; j++)
      if (answer[j] == answer[i]) {
        hints[i]++;
      }
  }
  // find correct digit and drop out from hint
  for (int i = 0; i < DIGIT_COUNT; i++) {
    char *ptr = strchr(answer, input[i]);
    if (input[i] == answer[i]) {
      hints[ptr - answer]--;
    }
  }
  // show hint
  for (int i = 0; i < DIGIT_COUNT; i++) {
    char *ptr = strchr(answer, input[i]);
    char data = INCORRECTED;
    if (input[i] == answer[i]) {
      data = CORRECTED;
    } else if (ptr && hints[ptr - answer] > 0) {
      hints[ptr - answer]--;
      data = WRONG_POSITION;
    }
    sendData(data);
  }
}

// Timer/Counter0 overflow interrupt
ISR(TIMER1_OVF_vect) {}

int pseudorandom() {
  int num;

  srand(TCNT1);
  num = abs(rand() % 90000) + 10000;
  if (num < 0) {
    return (num * -1);
  }
  return (num);
}

void setup() {
  initLCD();
  initTimer();
  //  for keypad
  //  keypad - col
  COL_DDR = 0x0F;
  // keypad - row
  ROW_PORT = 0xF0;

  sendCommand(CLEAR_DISPLAY);
  sendCommand(RETURN_HOME);
}

void loop() {

  if (!is_guess) {
    digit = mainScreen();
  }
  _delay_ms(20);
  if (digit == '#' && !is_guess) {
    randomNumber = pseudorandom();
    sprintf(answer, "%d", randomNumber);
    is_guess = 1;
    is_correct = 0;
    guess_count = 0;
  }

  i = 0;
  if (is_guess) {
    guess(&i, &guess_count);
  }
  if (i == DIGIT_COUNT && strcmp(answer, input) == 0) {
    is_guess = 0;
    is_correct = 1;
  } else if (i == DIGIT_COUNT) {
    hint();
  }

  if (guess_count == GUESS_COUNT) {
    is_guess = 0;
  }
}