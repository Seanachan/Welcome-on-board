#include "oled.h"

// =====================================================
//   I2C BIT-BANG (FIXED SDA GLITCH + TRUE OPEN-DRAIN)
// =====================================================

static void i2c_delay(void)
{
    __delay_us(4);
}

static void i2c_init_pins(void)
{
    OLED_SCL_TRIS = 0;
    OLED_SDA_TRIS = 0;
    OLED_SCL_LAT = 1;
    OLED_SDA_LAT = 1;
}

// SDA = input ? HIGH (released)
// SDA = output 0 ? LOW
static inline void sda_high(void)
{
    OLED_SDA_TRIS = 1;
}
static inline void sda_low(void)
{
    OLED_SDA_TRIS = 0;
    OLED_SDA_LAT = 0;
}

static inline void scl_high(void)
{
    OLED_SCL_LAT = 1;
}
static inline void scl_low(void)
{
    OLED_SCL_LAT = 0;
}

static void i2c_start(void)
{
    sda_high();
    scl_high();
    i2c_delay();
    sda_low();
    i2c_delay();
    scl_low();
}

static void i2c_stop(void)
{
    sda_low();
    scl_high();
    i2c_delay();
    sda_high();
    i2c_delay();
}

static uint8_t i2c_write_byte(uint8_t b)
{
    for (int i = 7; i >= 0; i--)
    {
        scl_low();
        if ((b >> i) & 1)
            sda_high();
        else
            sda_low();
        i2c_delay();
        scl_high();
        i2c_delay();
    }

    // ACK
    scl_low();
    sda_high(); // release
    i2c_delay();
    scl_high();
    i2c_delay();
    uint8_t ack = (OLED_SDA_PORT == 0);
    scl_low();
    return ack;
}

// =========================================
//   Low-level OLED I2C write
// =========================================

static void oled_i2c_write(uint8_t control, const uint8_t *data, uint16_t len)
{
    i2c_start();
    i2c_write_byte((OLED_ADDR << 1) | 0);
    i2c_write_byte(control);
    for (uint16_t i = 0; i < len; i++)
        i2c_write_byte(data[i]);
    i2c_stop();
}

static void oled_cmd(uint8_t c)
{
    oled_i2c_write(0x00, &c, 1);
}

static void oled_data(uint8_t d)
{
    oled_i2c_write(0x40, &d, 1);
}

static void oled_stream(const uint8_t *buf, uint16_t len)
{
    const uint16_t CHUNK = 64;
    uint16_t p = 0;

    while (p < len)
    {
        uint16_t n = (len - p > CHUNK) ? CHUNK : (len - p);
        i2c_start();
        i2c_write_byte((OLED_ADDR << 1) | 0);
        i2c_write_byte(0x40);
        for (uint16_t i = 0; i < n; i++)
        {
            i2c_write_byte(buf[p + i]);
        }
        i2c_stop();
        p += n;
    }
}

// =====================================================
//   SSD1306 INIT (fixed scan direction / correct image)
// =====================================================

void OLED_Init(void)
{
    i2c_init_pins();
    __delay_ms(100);

    oled_cmd(0xAE); // display off

    oled_cmd(0x20);
    oled_cmd(0x00); // horizontal addr
    oled_cmd(0xB0); // page 0

    // *** IMPORTANT: FIXED SCAN DIRECTION (NO ROTATION) ***
    oled_cmd(0xA0); // SEG normal
    oled_cmd(0xC0); // COM normal (top?bottom)

    oled_cmd(0xA8);
    oled_cmd(0x3F); // 64 rows
    oled_cmd(0xD3);
    oled_cmd(0x00);
    oled_cmd(0x40);

    oled_cmd(0x8D);
    oled_cmd(0x14);
    oled_cmd(0x81);
    oled_cmd(0x7F);
    oled_cmd(0xD9);
    oled_cmd(0xF1);
    oled_cmd(0xDB);
    oled_cmd(0x40);

    oled_cmd(0xA4);
    oled_cmd(0xA6);

    oled_cmd(0xAF);

    OLED_Clear();
}

// =====================================================
//   CLEAR SCREEN (100% CLEAN, NO FLICKER / NO DOTS)
// =====================================================

void OLED_Clear(void)
{
    uint8_t line[128];
    for (int i = 0; i < 128; i++)
        line[i] = 0x00;

    for (uint8_t page = 0; page < 8; page++)
    {
        oled_cmd(0xB0 | page);
        oled_cmd(0x00);
        oled_cmd(0x10);
        oled_stream(line, 128);
    }
}

// =====================================================
//   DRAW FULL 128×64 BITMAP
//   expects 1024 bytes: page0, page1, ... page7
// =====================================================

void OLED_DrawBitmap(const uint8_t *b)
{
    for (uint8_t page = 1; page < 7; page++)
    {
        oled_cmd(0xB0 | page);
        uint8_t col = 33;
        oled_cmd(0x00 | (col & 0x0F));        // low nibble
        oled_cmd(0x10 | ((col >> 4) & 0x0F)); // high nibble
        oled_stream(&b[page * 128 + 33], 64);
    }
}

void OLED_DisplayOn(void) { oled_cmd(0xAF); }
void OLED_DisplayOff(void) { oled_cmd(0xAE); }
void OLED_SetContrast(uint8_t c)
{
    oled_cmd(0x81);
    oled_cmd(c);
}
