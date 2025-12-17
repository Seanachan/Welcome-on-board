#ifndef DFPLAYER_H
#define DFPLAYER_H
static int volume;
void UART_WriteByte(unsigned char b);
void DF_SendCommand(unsigned char cmd, unsigned int param);
void DF_Init(void);
void DF_PlayTrack(unsigned int track);
void DF_Stop(void);
void DF_Volume(int vol_change);
#endif // DFPLAYER_H