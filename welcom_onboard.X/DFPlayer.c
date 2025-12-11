#include <xc.h>
#include "DFPlayer.h"
#define _XTAL_FREQ 4000000
void UART_WriteByte(unsigned char b)
{
  while (!TXSTAbits.TRMT)
    ; //
  TXREG = b;
}

void DF_SendCommand(unsigned char cmd, unsigned int param)
{
  unsigned char buf[6] = {
      0xFF, 0x06, cmd, 0x00,
      (unsigned char)(param >> 8),
      (unsigned char)(param & 0xFF)};

  unsigned int sum = 0;
  for (int i = 0; i < 6; i++)
    sum += buf[i];
  unsigned int checksum = 0xFFFF - sum + 1;

  UART_WriteByte(0x7E); // start
  for (int i = 0; i < 6; i++)
    UART_WriteByte(buf[i]);
  UART_WriteByte((unsigned char)(checksum >> 8));
  UART_WriteByte((unsigned char)(checksum & 0xFF));
  UART_WriteByte(0xEF); // end

  __delay_ms(50);
}

void DF_Init(void)
{
  DF_SendCommand(0x06, 20); // set volume = 20
}

void DF_PlayTrack1(void)
{
  DF_SendCommand(0x03, 1); // Play 0001.mp3
}
void DF_Stop(void)
{
  DF_SendCommand(0x16, 0); // Stop playing
}
void DF_Volume(int vol_change)
{
  if (volume + vol_change < 0)
    volume = 0;
  if (volume > 30)
    volume = 30;
  volume = volume + vol_change;
  DF_SendCommand(0x06, volume); // set volume
}