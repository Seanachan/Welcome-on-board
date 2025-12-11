#ifndef DFPLAYER_H
#define DFPLAYER_H
void UART_WriteByte(unsigned char b);
void DF_SendCommand(unsigned char cmd, unsigned int param);
void DF_Init(void);
void DF_PlayTrack1(void);
void DF_Stop(void);
void DF_Volume(int vol_change);
#endif // DFPLAYER_H