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
#pragma config PBADEN = OFF   // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
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

// Standard includes
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <xc.h>

// Self includes
#include "bluetooth.h"
#include "DFPlayer.h"
// #include "UART.h"
#include "motor.h"
#include "SPI.h"
#include "seg7/seg7.h"

// Definitions
#define _XTAL_FREQ 4000000
#define STR_MAX 100
#define VR_MAX ((1 << 10) - 1)
char buffer[STR_MAX];
int buffer_size = 0;
bool btn_interr = false;
// Global Variables
const int array[] = {0, 40, 50, 60};

int state = -1;
long long adc_sum = 0;
int sum_cnt = 0;
void putch(char data)
{ // Output on Terminal
    if (data == '\n' || data == '\r')
    {
        while (!TXSTAbits.TRMT)
            ;
        TXREG = '\r';
        while (!TXSTAbits.TRMT)
            ;
        TXREG = '\n';
    }
    else
    {
        while (!TXSTAbits.TRMT)
            ;
        TXREG = data;
    }
}

void ClearBuffer()
{
    for (int i = 0; i < STR_MAX; i++)
        buffer[i] = '\0';
    buffer_size = 0;
}

void MyusartRead()
{
    char data = RCREG;
    if (!isprint(data) && data != '\r')
        return;
    buffer[buffer_size++] = data;
    putch(data);
}

int GetString(char *str)
{
    if (buffer[buffer_size - 1] == '\r')
    {
        buffer[--buffer_size] = '\0';
        strcpy(str, buffer);
        ClearBuffer();
        return 1;
    }
    else
    {
        str[0] = '\0';
        return 0;
    }
}
void __interrupt(high_priority) Hi_ISR(void)
{

    if (PIR1bits.ADIF)
    {
        long long value = (ADRESH << 8) | ADRESL;
        // do sth
        adc_sum += value;
        sum_cnt++;
        if (sum_cnt >= 20)
        {
            long long average_value = adc_sum / 20;
            seg7_displayNumber(average_value);
            // __delay_ms(500);
            adc_sum = 0;
            sum_cnt = 0;
        }

        PIR1bits.ADIF = 0; // clear flag bit

        // step5 & go back step3
        // __delay_ms(3); // delay at least 2tad (4M ver.)
        ADCON0bits.GO = 1;
    }

    return;
}
void __interrupt(low_priority) Lo_ISR(void)
{
    if (RCIF)
    {
        if (RCSTAbits.OERR)
        {
            CREN = 0;
            Nop();
            CREN = 1;
        }

        MyusartRead();
    }
    return;
}

void keyboard_input(char *str)
{
    for (int i = 0; i < strlen(str); i++)
        str[i] = toupper(str[i]);
    printf("BT CMD: %s\n", str);

    // __delay_ms(30);

    if (strcmp(str, "FORWARD") == 0)
    {
        // move car forward
        forward();
    }
    else if (strcmp(str, "REVERSE") == 0)
    {
        // move car backward
        backward();
    }
    else if (strcmp(str, "PARK") == 0)
    {
        // park the car
        park();
    }
    else if (strcmp(str, "STRAIGHT") == 0)
    {
        // steering straight
    }
    else if (strcmp(str, "TURN_LEFT") == 0)
    {
        // steering left
        turnLeft();
    }
    else if (strcmp(str, "TURN_RIGHT") == 0)
    {
        // steering right
        turnRight();
    }
    else if (strcmp(str, "HIGH_SPEED") == 0)
    {
        // set high speed
        highSpeed();
    }
    else if (strcmp(str, "LOW_SPEED") == 0)
    {
        // set low speed
        lowSpeed();
    }
    else if (strcmp(str, "PLAY_MUSIC") == 0)
    {
        // printf("Play music\n");
        DF_PlayTrack1(); // Play track 1
    }
    else if (strcmp(str, "STOP_MUSIC") == 0)
    {
        // printf("Stop music\n");
        DF_Stop(); // Stop music
    }
    else if (strcmp(str, "VOL_UP") == 0)
    {
        DF_Volume(5);
    }
    else if (strcmp(str, "VOL_DOWN") == 0)
    {
        DF_Volume(-5);
    }
    else
    {
        printf("Unknown CMD: %s\n", str);
    }
    // __delay_ms(50);
}

void main(void)
{
    OSCCONbits.IRCF = 0b110; // 4 MHz
    ADCON1 = 0x0F;
    CCP_Seg7_Initialize();
    // SPI_Init();
    // MFRC522_Init();
    // Initialize_UART();

    TRISCbits.TRISC6 = 0; // TX as output
    TRISCbits.TRISC7 = 1; // RX as input
    LATCbits.LATC6 = 0;   // Idle state high
    LATCbits.LATC7 = 0;   // Idle state high

    // Baud rate ~9600 @ 4MHz using 16-bit BRG
    TXSTAbits.SYNC = 0;    // Asynchronous
    BAUDCONbits.BRG16 = 0; // 8-bit Baud Rate Generator
    TXSTAbits.BRGH = 1;    // High speed
    // SPBRGH = 0;
    SPBRG = 25; // 4MHz -> 9600 bps

    // Serial enable
    RCSTAbits.SPEN = 1; // Enable async serial port
    PIR1bits.TXIF = 0;  // Clear TX flag
    PIR1bits.RCIF = 0;  // Clear RX flag
    TXSTAbits.TXEN = 1; // Enable transmission
    RCSTAbits.CREN = 1; // Enable continuous receive
    PIE1bits.TXIE = 0;  // Disable TX interrupt
    IPR1bits.TXIP = 0;  // TX interrupt priority
    PIE1bits.RCIE = 1;  // Enable RX interrupt
    IPR1bits.RCIP = 0;  // RX interrupt priority (low)
    // DF_Init();
    seg7_setBrightness(7);
    seg7_display4(gear[0][0], gear[0][1], gear[0][2], gear[0][3]);
    __delay_ms(500);
    unsigned char uid[7], uidLen;
    unsigned char status;

    while (1)
    {
        // __delay_ms(100);
        // CCP
        CCPR1L = speed;
        CCP1CONbits.DC1B = 0;
        CCPR2L = speed;
        CCP2CONbits.DC2B = 0;

        if(PN532_ReadUID(uid, &uidLen)) {
            
            
            // 【修改這裡】
            // 你的卡片是 7 bytes (或 8 bytes)，所以不能檢查 == 4
            // 直接比對前幾個獨特的號碼即可 (04 B3 31 4E)
            if(uid[1]==0xB3 && uid[2]==0x31 && uid[3]==0x4E) {
                //printf("Match! Play Music.\r\n");
                DF_PlayTrack1();
            }
            
            __delay_ms(1000); // 讀到卡後暫停一秒
        } else {
            __delay_ms(50); // 沒讀到卡稍微休息
        }
        
    }
}
