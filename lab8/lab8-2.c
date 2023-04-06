#define F_CPU 8000000L

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

/*
Lab 8.2 AVR Timer: Input Capture

UltraSonic Sensor:
Design a circuit and program to measure the distance of an object using the
ultrasonic sensor. HC-SR04 then display the distance on the LCD in centimeters.
*/

// LCD
#define COMMAND_PORT PORTB
#define COMMAND_DDR DDRB
#define DATA_PORT PORTD
#define DATA_DDR DDRD

#define RS _BV(PB1)
#define E _BV(PB2)

#define DB4 _BV(PD0)
#define DB5 _BV(PD1)
#define DB6 _BV(PD2)
#define DB7 _BV(PD3)

#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02
#define SET_CURSOR 0x80

// Sensor
#define SENSOR_PORT PORTC
#define SENSOR_DDR DDRC

#define TRIG _BV(PC5)
#define ECHO _BV(PB0)

// Constants
#define CONVERSION_FACTOR 0.034 / 2
// 0.034 cm per microsecond
// 2 for round trip

// Global Variables
char buffer[16] = "";
volatile uint16_t captureValue = 0;
volatile uint8_t captureFlag = 0;
volatile uint16_t distance = 0;

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

void initTimer1() {
  // Set the timer to normal mode
  TCCR1A = 0x00;

  // Set the timer:
  TCCR1B = _BV(CS11)     // Prescaler of 8
           | _BV(ICES1)  // Trigger on rising edge
           | _BV(ICNC1); // Noise canceler

  // Enable input capture interrupt
  TIMSK1 = _BV(ICIE1);
}

void initSensor() {
  // Set up the sensor port
  SENSOR_DDR |= TRIG;
  SENSOR_DDR &= ~ECHO;

  // Set the trigger to low
  SENSOR_PORT &= ~TRIG;
}

ISR(TIMER1_CAPT_vect) {
  if (captureFlag == 0) {
    // Set the timer to trigger on a falling edge
    TCCR1B &= ~_BV(ICES1);

    // Set the flag
    captureFlag = 1;

    // Capture the value
    captureValue = ICR1;

  } else {
    // Set the timer to trigger on a rising edge
    TCCR1B |= _BV(ICES1);

    // Clear the flag
    captureFlag = 0;

    captureValue = ICR1 - captureValue;

    // Calculate the distance
    distance = (uint16_t)(captureValue * CONVERSION_FACTOR);
  }
}

void setup() {

  // Set up the LCD
  initLCD();

  // Set up the sensor
  initSensor();

  // Set up Timer1
  initTimer1();

  // Enable interrupts
  sei();
}

void loop() {
  // Send a 10us pulse to the sensor
  SENSOR_PORT |= TRIG;
  _delay_us(10);
  SENSOR_PORT &= ~TRIG;

  sendCommand(CLEAR_DISPLAY);
  sendCommand(SET_CURSOR);
  // Display the distance
  sprintf(buffer, "Distance %u cm.", distance);
  sendString(buffer);

  // Wait 1 second
  _delay_ms(1000);
}

int main() {
  setup();
  while (1) {
    loop();
  }
}