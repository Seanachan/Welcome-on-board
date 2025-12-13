#ifndef seg7_H
#define seg7_H

#include <xc.h>
#include <stdint.h>

// ===== ???? =====
#define _XTAL_FREQ 4000000  // 8MHz?????????

// ===== ???? =====
// CLK ????????
#define seg7_CLK_LAT LATAbits.LATA0
#define seg7_CLK_TRIS TRISAbits.TRISA0

// DIO ??????
#define seg7_DIO_LAT LATAbits.LATA1
#define seg7_DIO_TRIS TRISAbits.TRISA1
#define seg7_DIO_PORT PORTAbits.RA1  // ??? ACK

// ===== function declaration =====
void seg7_init(void);                         // init pin
void seg7_start(void);                        // start signal
void seg7_stop(void);                         // stop signal
void seg7_writeByte(uint8_t b);              // transfer 1 byte
void seg7_setBrightness(uint8_t brightness); // set brightness?0~7?
void seg7_displayDigit(uint8_t pos, uint8_t digit); // display specific digit
void seg7_displayNumber(int num);            // display 0~9999 integer
void seg7_display4(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
void seg7_scroll(const uint8_t *buf, uint8_t len);
void seg7_display_fail();
void seg7_display_successful();

static const uint8_t gear[4][4] = {
    {0x73, 0x00, 0x00, 0x00}, // P
    {0x00, 0x7C, 0x00, 0x00}, // b
    {0x00, 0x00, 0x76, 0x00}, // H
    {0x00, 0x00, 0x00, 0x38}  // L
};

static const uint8_t segCode[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

const uint8_t SUCCESS_BUF[] = {
    0x00, // blank
    0x6D, // S
    0x3E, // U
    0x39, // C
    0x39, // C
    0x79, // E
    0x6D, // S
    0x6D, // S
    0x00  // blank
};

const uint8_t FAIL_BUF[] = {
    0x00, // blank
    0x71, // F
    0x77, // A
    0x06, // I
    0x38, // L
    0x00  // blank
};

#endif
