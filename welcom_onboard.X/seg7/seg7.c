#include "seg7.h"


void seg7_start() {
    seg7_DIO_TRIS = 0;  // DIO = output
    seg7_DIO_LAT = 1;
    seg7_CLK_LAT = 1;
    __delay_us(3);
    seg7_DIO_LAT = 0;
}

void seg7_stop() {
    seg7_DIO_TRIS = 0;
    seg7_CLK_LAT = 0;
    __delay_us(3);
    seg7_CLK_LAT = 1;
    seg7_DIO_LAT = 1;
}

void seg7_writeByte(uint8_t b) {
    for(uint8_t i=0; i<8; i++) {
        seg7_CLK_LAT = 0;
        __delay_us(3);
        seg7_DIO_TRIS = 0;  // output
        seg7_DIO_LAT = (b & 0x01);
        b >>= 1;

        __delay_us(3);
        seg7_CLK_LAT = 1;
        __delay_us(3);
    }

    // ACK
    seg7_CLK_LAT = 0;
    seg7_DIO_TRIS = 1; // input
    __delay_us(5);

    seg7_CLK_LAT = 1;
    __delay_us(5);
    seg7_CLK_LAT = 0;


}

void seg7_init() {
    seg7_CLK_TRIS = 0;
    seg7_DIO_TRIS = 0;


    seg7_CLK_LAT = 1;
    seg7_DIO_LAT = 1;
    seg7_displayNumber(0);

}

void seg7_setBrightness(uint8_t brightness) {
    // brightness: 0~7
    seg7_start();
    seg7_writeByte(0x88 + (brightness & 0x07));
    seg7_stop();
}

void seg7_displayDigit(uint8_t pos, uint8_t digit) {
    seg7_start();
    seg7_writeByte(0x40);  // automatically increament address
    seg7_stop();


    seg7_start();
    seg7_writeByte(0xC0 + pos);
    seg7_writeByte(segCode[digit]);
    seg7_stop();


}

void seg7_display4(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
    seg7_start();
    seg7_writeByte(0x40); 
    seg7_stop();

    seg7_start();
    seg7_writeByte(0xC0);

    seg7_writeByte(d0);
    seg7_writeByte(d1);
    seg7_writeByte(d2);
    seg7_writeByte(d3);

    seg7_stop();
}

void seg7_displayNumber(int num) {
    seg7_start();
    seg7_writeByte(0x40);
    seg7_stop();


    seg7_start();
    seg7_writeByte(0xC0);

    if(num > 9999) num = 9999;
    if(num < 0) num = 0;

    seg7_writeByte(segCode[num / 1000]);
    seg7_writeByte(segCode[(num / 100) % 10]);
    seg7_writeByte(segCode[(num / 10) % 10]);
    seg7_writeByte(segCode[num % 10]);

    seg7_stop();


}

void seg7_scroll(const uint8_t *buf, uint8_t len) {
    uint8_t window[4];

    for (int pos = 0; pos <= len - 4; pos++) {
        window[0] = buf[pos];
        window[1] = buf[pos+1];
        window[2] = buf[pos+2];
        window[3] = buf[pos+3];

        seg7_display4(window[0], window[1], window[2], window[3]);
        __delay_ms(300);
    }
}

void display_fail(){
    seg7_scroll(FAIL_BUF, 6);
}

void display_success(){
    seg7_scroll(SUCCESS_BUF, 9);
}