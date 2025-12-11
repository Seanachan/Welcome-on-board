#ifndef OLED_H
#define OLED_H

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 4000000

// I2C bit-bang pins (change if needed)
// fixed
#define OLED_SCL_LAT LATCbits.LATC3
#define OLED_SCL_TRIS TRISCbits.TRISC3
#define OLED_SDA_LAT LATCbits.LATC4
#define OLED_SDA_TRIS TRISCbits.TRISC4
#define OLED_SDA_PORT PORTCbits.RC4

// SSD1306 I2C address (7-bit <<1 for write in code we use 0x3C as device addr)
#define OLED_ADDR 0x3C

// API
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawBitmap(const uint8_t *bitmap); // bitmap must be 1024 bytes (page order)
void OLED_DisplayOn(void);
void OLED_DisplayOff(void);
void OLED_SetContrast(uint8_t contrast);

#endif
