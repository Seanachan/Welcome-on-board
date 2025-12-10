#ifndef SOFTWARE_UART_H
#define SOFTWARE_UART_H
#include <xc.h>
// --- Config for Software UART Pin ---
// --- Software UART Functions ---
void SoftUART_Init(void);
void SoftUART_Write(unsigned char data);

// --- DFPlayer Logic (Uses SoftUART) ---
void Send_DFPlayer_Command(unsigned char command, unsigned int parameter);

void mp3_Init();
void mp3_Play(int track);
void mp3_Stop();
#endif // SOFTWARE_UART_Hp3_Stop();