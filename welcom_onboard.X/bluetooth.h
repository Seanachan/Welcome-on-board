#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <xc.h>

// Function prototypes
void keyboard_input(char *str);
void DF_Init(void);
void DF_SendCommand(unsigned char cmd, unsigned int param);
void DF_PlayTrack1(void);
void DF_Stop(void);
void DF_Volume(int vol_change);
void putch(char data);
int GetString(char *str);
void MyusartRead(void);
void ClearBuffer(void);
void UART_WriteByte(unsigned char b);
#endif // BLUETOOTH_H