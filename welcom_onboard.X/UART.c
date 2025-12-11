#include <xc.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "UART.h"
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
  OSCCONbits.IRCF = 0b110; // 4 MHz
  // --- UART Pin Configuration ---
  // Note: EUSART module automatically handles direction when SPEN=1,
  // but setting TRIS to 1 is standard practice for PIC18 EUSART.
  TRISCbits.TRISC6 = 1; // RC6(TX)
  TRISCbits.TRISC7 = 1; // RC7(RX)

  // --- Baud Rate Configuration ---
  // Baud rate = 9600 (Calculated for Fosc = 4MHz)
  TXSTAbits.SYNC = 0;    // Asynchronous mode
  BAUDCONbits.BRG16 = 0; // 8-bit Baud Rate Generator
  TXSTAbits.BRGH = 1;    // High Baud Rate Select
  SPBRG = 25;            // Control the period

  // --- Serial Module Enable ---
  RCSTAbits.SPEN = 1; // Enable serial port (Configures RX/TX pins)

  // --- Transmitter Setup ---
  PIR1bits.TXIF = 0;  // Clear TX Flag (Note: Usually read-only, cleared by hardware)
  TXSTAbits.TXEN = 1; // Enable transmission
  PIE1bits.TXIE = 0;  // Disable Transmit Interrupt
  IPR1bits.TXIP = 0;  // Transmit Interrupt Priority

  // --- Receiver Setup ---
  PIR1bits.RCIF = 0;  // Clear RC Flag (Note: Usually read-only, cleared by reading RCREG)
  RCSTAbits.CREN = 1; // Continuous receive enable
  PIE1bits.RCIE = 1;  // Enable Receive Interrupt
  IPR1bits.RCIP = 0;  // Receive Interrupt Priority
}