#ifndef UART_H
#define UART_H
void putch(char data);
void ClearBuffer();
void MyusartRead();
void Initialize_UART(void);
int GetString(char *str);
#endif // UART_H