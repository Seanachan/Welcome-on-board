// CONFIG 同你原本那一份就好，略...
// CONFIG1H
#pragma config OSC = INTIO67 // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF   // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = ON     // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3        // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 1 // Watchdog Timer Postscale Select bits (1:1)

// CONFIG3H
#pragma config CCP2MX = PORTC // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = ON    // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF  // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON     // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF   // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config XINST = OFF // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)
#define _XTAL_FREQ 4000000
#include <xc.h>
#include <stdio.h>

// ==== DFPlayer: 用 RC6 硬體 UART 送命令 ====

void UART_Init_9600(void)
{
    OSCCONbits.IRCF = 0b110; // 4 MHz
    ADCON1 = 0x0F;           // 全部腳位改成 digital

    TRISCbits.TRISC6 = 1; // TX pin
    TRISCbits.TRISC7 = 1; // RX pin (不用也沒關係)

    TXSTAbits.SYNC = 0;    // 非同步
    BAUDCONbits.BRG16 = 0; // 8-bit BRG
    TXSTAbits.BRGH = 1;    // high speed
    SPBRG = 25;            // 4MHz, 9600 baud

    RCSTAbits.SPEN = 1; // 開 UART
    TXSTAbits.TXEN = 1; // 允許傳送
    RCSTAbits.CREN = 1; // 允許接收
}

void UART_WriteByte(unsigned char b)
{
    while (!TXSTAbits.TRMT)
        ; // 等待移位暫存器空
    TXREG = b;
}

void DF_SendCommand(unsigned char cmd, unsigned int param)
{
    unsigned char buf[6] = {
        0xFF, 0x06, cmd, 0x00,
        (unsigned char)(param >> 8),
        (unsigned char)(param & 0xFF)};

    unsigned int sum = 0;
    for (int i = 0; i < 6; i++)
        sum += buf[i];
    unsigned int checksum = 0xFFFF - sum + 1;

    UART_WriteByte(0x7E); // start
    for (int i = 0; i < 6; i++)
        UART_WriteByte(buf[i]);
    UART_WriteByte((unsigned char)(checksum >> 8));
    UART_WriteByte((unsigned char)(checksum & 0xFF));
    UART_WriteByte(0xEF); // end

    __delay_ms(50);
}

void DF_Init(void)
{
    DF_SendCommand(0x06, 25); // volume = 25
}

void DF_PlayTrack1(void)
{
    DF_SendCommand(0x03, 1); // 播放 0001.mp3
}

void main(void)
{
    UART_Init_9600();

    __delay_ms(1000); // 等 DFPlayer 上電穩定
    DF_Init();

    __delay_ms(500);
    DF_PlayTrack1(); // 上電就播

    while (1)
    {
        // 什麼都不要做，等歌播完
    }
}