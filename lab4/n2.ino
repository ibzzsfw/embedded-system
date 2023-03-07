void commitData()
{
  // toggle E
  PORTD |= 1 << PORTD4;
  _delay_us(10);
  PORTD &= ~(1 << PORTD4);
  _delay_us(10);
}

void sendLCDcommand(uint8_t command)
{
  // pull RS low
  PORTD &= ~(1 << PORTD2);
  // send high nibble (half of 4-bit data) of the command
  PORTB &= 0xF0; // clear
  PORTB = command >> 4; // shift 4 to left to use just 0-3 channel
  commitData();
  
  
  // send low nibble (half of 4-bit data) of the command
  PORTB &= 0xF0; // clear
  PORTB |= (command & 0x0F); // first 4 bits = 0
  commitData();
}

void sendLCDdata(uint8_t command)
{
  // pull RS high
  PORTD |= (1 << PORTD2);
  // send high nibble (half of 4-bit data) of the command
  PORTB &= 0xF0; // clear
  PORTB = command >> 4; // shift 4 to left to use just 0-3 channel
  commitData();
  
  
  // send low nibble (half of 4-bit data) of the command
  PORTB &= 0xF0; // clear
  PORTB |= (command & 0x0F); // first 4 bits = 0
  commitData();
}

void lcdDisplayString(char *str)
{
  while(*str != '\0')
  {
    sendLCDdata(*str);
    str++;
  } 
}

void initLCD()
{ 
  DDRB |= 0x0F;
  PORTB &= 0xF0; // clear
  
  DDRD |= (1 << DDD2) |(1 << DDD4);
  PORTD &= ~(1 << DDD2) & ~(1 << DDD4); // clear
  
  sendLCDcommand(0x33);
  sendLCDcommand(0x32);
  sendLCDcommand(0x28);
  sendLCDcommand(0x0E);
  sendLCDcommand(0x01);
  sendLCDcommand(0x80 );
}


void USART_Init(unsigned int ubrr)
{
    /* Set baud rate */
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    /* Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    /* Set frame format: 8data, 2stop bit */
    UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit(unsigned char data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1<<UDRE0)));
    /* Put data into buffer, sends the data */
    UDR0 = data;
}

unsigned char USART_Receive()
{
    /* Wait for data to be received */
    while (!(UCSR0A & (1<<RXC0)));
    /* Get and return received data from buffer */
    return UDR0;
}

#include <string.h>
#define Baud 9600
#define My_Ubrr (F_CPU / (16 * Baud)) - 1

void setup()
{
  USART_Init(My_Ubrr);
  initLCD();
}

void loop()
{
  char name[100];
  char greeting[] = "Hello ";
  int i;
  
  i = 0;
  while(1) {
    name[i] = USART_Receive();
    if (name[i] == '.') {
      name[i] = '\0';
      break;
    }
    i++;
  }

  sendLCDcommand(0x01);
  lcdDisplayString(greeting);
  lcdDisplayString(name);
  memset(name, 0, sizeof(name));
}