#include <xc.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "UART.h"
static char buffer[STR_MAX];
static int buffer_size = 0;
// ---------------- Uart --------------------
void putch(char data)
{ // Output on Terminal
  if (data == '\n' || data == '\r')
  {
    while (!TXSTAbits.TRMT)
      ;
    TXREG = '\r';
    while (!TXSTAbits.TRMT)
      ;
    TXREG = '\n';
  }
  else
  {
    while (!TXSTAbits.TRMT)
      ;
    TXREG = data;
  }
}

void ClearBuffer()
{
  for (int i = 0; i < STR_MAX; i++)
    buffer[i] = '\0';
  buffer_size = 0;
}
void MyusartRead()
{
  char data = RCREG;
  if (!isprint(data) && data != '\r')
    return;
  buffer[buffer_size++] = data;
  putch(data);
}

int GetString(char *str)
{
  if (buffer[buffer_size - 1] == '\r')
  {
    buffer[--buffer_size] = '\0';
    strcpy(str, buffer);
    ClearBuffer();
    return 1;
  }
  else
  {
    str[0] = '\0';
    return 0;
  }
}
void Initialize_UART(void)
{
  // Clock assumed already set by caller (4 MHz)

  // Pin directions for UART
  TRISCbits.TRISC6 = 0; // TX as output
  TRISCbits.TRISC7 = 1; // RX as input
  LATCbits.LATC6 = 0;   // Idle state high
  LATCbits.LATC7 = 0;   // Idle state high

  // Baud rate ~9600 @ 4MHz using 16-bit BRG
  TXSTAbits.SYNC = 0;    // Asynchronous
  BAUDCONbits.BRG16 = 0; // 16-bit Baud Rate Generator
  TXSTAbits.BRGH = 1;    // High speed
  // SPBRGH = 0;
  SPBRG = 25; // 4MHz -> 9600 bps

  // Serial enable
  RCSTAbits.SPEN = 1; // Enable async serial port
  PIR1bits.TXIF = 0;  // Clear TX flag
  PIR1bits.RCIF = 0;  // Clear RX flag
  TXSTAbits.TXEN = 1; // Enable transmission
  RCSTAbits.CREN = 1; // Enable continuous receive
  PIE1bits.TXIE = 0;  // Disable TX interrupt
  IPR1bits.TXIP = 0;  // TX interrupt priority
  PIE1bits.RCIE = 1;  // Enable RX interrupt
  IPR1bits.RCIP = 0;  // RX interrupt priority (low)
}
