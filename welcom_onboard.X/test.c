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

#include <xc.h>
#include <stdio.h>

#define _XTAL_FREQ 4000000

// 最簡單版 putch：printf 會用到
void putch(char c)
{
    while (!TXSTAbits.TRMT)
        ; // 等待 TX shift register 空
    TXREG = c;
}

void UART_Init(void)
{
    // RC6 = TX, RC7 = RX 給 EUSART 用
    TRISCbits.TRISC6 = 1; // 讓周邊接管 TX
    TRISCbits.TRISC7 = 1; // RX input

    TXSTAbits.SYNC = 0;    // 非同步
    BAUDCONbits.BRG16 = 0; // 8-bit Baud
    TXSTAbits.BRGH = 0;    // 高速模式

    // 9600 @ 4MHz: SPBRG ≈ 25
    SPBRG = 51;
    SPBRGH = 0;

    RCSTAbits.SPEN = 1; // 啟用串列埠 (RC6/RC7)
    TXSTAbits.TXEN = 1; // 開啟傳送
    RCSTAbits.CREN = 1; // 開啟接收（雖然這個測試沒用到）
}

void OSC_Init(void)
{
    // 4MHz 內部振盪
    OSCCONbits.IRCF = 0b110;
}

void USART_char(char byte)
{
    TXREG = byte;
    while (!TXIF)
        ;
    while (!TRMT)
        ;
}
// End of function//

// Function to Load buffer with string//
void USART_string(char *string)
{
    while (*string)
        USART_char(*string++);
}
// End of function//

// Function to send data from RX. buffer//
void send_data()
{
    TXREG = 13;
    __delay_ms(500);
}
// End of function//

// Function to get a char from buffer of Bluetooth//
char USART_get_char(void)
{
    if (OERR) // check for over run error
    {
        CREN = 0;
        CREN = 1; // Reset CREN
    }

    if (RCIF == 1) // if the user has sent a char return the char (ASCII value)
    {
        while (!RCIF)
            ;
        return RCREG;
    }
    else // if user has sent no message return 0
        return 0;
}
// End of function/

void main(void)
{
    // Scope variable declarations//
    int get_value;
    // End of variable declaration//

    // I/O Declarations//
    TRISA = 0;
    // End of I/O declaration//

    UART_Init(); // lets get our bluetooth ready for action
    OSC_Init();  // Initialize oscillator

    // Show some introductory message once on power up//
    USART_string("Bluetooth iS Ready");
    send_data();
    USART_string("Press 1 to turn ON LED");
    send_data();
    USART_string("Press 0 to turn OFF LED");
    send_data();
    // End of message//

    while (1) // The infinite lop
    {

        get_value = USART_get_char(); // Read the char. received via BT

        // If we receive a '0'//
        if (get_value == '0')
        {
            LATAbits.LATA1 = 0;
            USART_string("LED turned OFF");
            send_data();
        }

        // If we receive a '1'//
        if (get_value == '1')
        {
            LATAbits.LATA1 = 1;
            USART_string("LED turned ON");
            send_data();
        }
    }
}