#ifndef UART_H
#define UART_H
#define STR_MAX 100
static char buffer[STR_MAX];
static int buffer_size;
void putch(char data);
void ClearBuffer();
void MyusartRead();
void Initialize_UART(void);
int GetString(char *str);
#endif // UART_H
