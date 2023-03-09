// Question 5
// Suppakorn Rakna
// 63070501061

/*
Keypad layout
1 2 3 A
4 5 6 B
7 8 9 C
* 0 # D

Keypad : port mapping
row1..4 : DDD7..4
col1..4 : DDD3..0

LCD : port mapping
RS : PORTB4
E : PORTB5
DB4..7 : PORTB3..0
*/

/*
#Problem statement:
Connect ATmega328P to a 16x2 LCD display and a 4x4 keypad.
For each key pressed on the keypad, display the corresponding character on the
LCD.

#Requirmenets:
The microcontroller has 2 functions:
1. Set up a password:
- press #9999* to enter the password setup mode
- enter any key (4 digits) as a password

2. Enter the password:
- press 4 digits and end with * to enter the password, the program will check if
it matches the password set in step 1
- if the password is correct, display "Pass" on the LCD, otherwise display
"Access Denied"

hint: default password is 0000
*/

// define for LCD
#define COMMAND_PORT PORTC
#define COMMAND_DDR DDRC
#define RS (1 << PORTC4)
#define E (1 << PORTC5)

#define DATA_PORT PORTB
#define DATA_DDR DDRB

// LCD commands
#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02

// utility constants
#define PASSWORD_LENGTH 4
#define MAX_INPUT_LENGTH 6

char password[PASSWORD_LENGTH + 1] = "0000";
char setup_mode[] = "#9999*";
const unsigned char keypad[4][4] = {{'1', '2', '3', 'A'},
                                    {'4', '5', '6', 'B'},
                                    {'7', '8', '9', 'C'},
                                    {'*', '0', '#', 'D'}};

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

  const uint8_t lcd_init_cmds[] = {0x33, 0x32, 0x28, 0x0E, 0x01, 0x80};
  const int cmds_count = 6;

  DDRB |= 0x0f;
  PORTB &= 0xF0;

  DDRD |= (1 << DDD2) | (1 << DDD4);
  PORTD &= ~(1 << PORTD2) & ~(1 << PORTD4);

  for (int i = 0; i < cmds_count; i++) {
    sendCommand(lcd_init_cmds[i]);
  }
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
  // listennig to a key press event

  unsigned char col = 0x01;
  unsigned char row;

  while ((PIND & 0xF0) == 0xF0) {
    col = (col == 0x10) ? 0x01 : (col << 1); // shift left
    PORTD = 0xF0 | ~(col & 0x0F);            // set col by active low
    _delay_ms(10);                           // delay
    row = (PIND & 0xF0);                     // get row
  }
  _delay_ms(200); // delay

  unsigned char x = ~(row >> 4) + '@';
  unsigned char y = col + '0';

  // get the digit from keypad layout
  return (mapping(x, y));
}

void setPassword() {
  for (int i = 0; i < PASSWORD_LENGTH; i++) {
    password[i] = onKeyPress();
    sendData(password[i]);
  }
}

int stringCompare(char *str1, char *str2, int len) {
  // len use for specify the length of string to compare in case of
  // string with different length, prevent from accessing out of bound
  for (int i = 0; i < len; i++) {
    if (str1[i] != str2[i]) {
      return (0);
    }
  }
  return (1);
}

void setup() {
  // for LCD
  initLCD();
  // for keypad
  // keypad - col
  DDRD = 0x0F;
  // keypad - row
  PORTD = 0xF0;
  // led - col
  DDRB = 0x0F;
  // led - row
  DDRC = 0x0F;
}

void loop() {
  // clear display and return home when start
  sendCommand(CLEAR_DISPLAY);
  sendCommand(RETURN_HOME);

  char input[MAX_INPUT_LENGTH]; // fixed length
  int length = 0;               // current length
  int i = 0;

  // while not end with * or not exceeding limit, keep looping and get input
  for (i = 0; i < MAX_INPUT_LENGTH; i++) {
    char digit = onKeyPress();
    sendData(digit);
    input[length++] = digit; // append to input, '*' will be appended too
    if (digit == '*') {
      break;
    }
  }

  // clear display and return home when enter
  sendCommand(CLEAR_DISPLAY);
  sendCommand(RETURN_HOME);

  if (stringCompare(input, setup_mode, 6) == 1) {
    setPassword();
  } else {
    if (stringCompare(input, password, PASSWORD_LENGTH) == 1) {
      sendString("Pass");
    } else {
      sendString("Access Denied");
    }
  }
  _delay_ms(500);
}