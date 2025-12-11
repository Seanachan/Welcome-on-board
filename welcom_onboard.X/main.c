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

// ==========================================
//  INCLUDES & FREQUENCY
// ==========================================
#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define _XTAL_FREQ 4000000 // 4 MHz Crystal/Oscillator

// ==========================================
//  PIN DEFINITIONS
// ==========================================

// --- RC522 RFID Pins ---
#define MFRC522_CS_PIN LATAbits.LATA0 // Chip Select (SDA)
#define MFRC522_CS_TRIS TRISAbits.TRISA0

#define MFRC522_RST_PIN LATEbits.LATE0 // Reset (RST)
#define MFRC522_RST_TRIS TRISEbits.TRISE0

// --- DFPlayer Mini (MP3) Pins ---
// Note: You defined LATB1 in your snippet.
// Ensure your physical wire is on RB1.
#define MP3_TX_PIN LATBbits.LATB1
#define MP3_TX_TRIS TRISBbits.TRISB1

// ==========================================
//  RC522 REGISTER DEFINITIONS
// ==========================================
#define MFRC522_REG_COMMAND 0x01
#define MFRC522_REG_COMM_IEN 0x02
#define MFRC522_REG_DIV_IEN 0x03
#define MFRC522_REG_COMM_IRQ 0x04
#define MFRC522_REG_DIV_IRQ 0x05
#define MFRC522_REG_ERROR 0x06
#define MFRC522_REG_STATUS1 0x07
#define MFRC522_REG_STATUS2 0x08
#define MFRC522_REG_FIFO_DATA 0x09
#define MFRC522_REG_FIFO_LEVEL 0x0A
#define MFRC522_REG_CONTROL 0x0C
#define MFRC522_REG_BIT_FRAMING 0x0D
#define MFRC522_REG_COLL 0x0E
#define MFRC522_REG_MODE 0x11
#define MFRC522_REG_TX_ASK 0x15
#define MFRC522_REG_TX_CONTROL 0x14
#define MFRC522_REG_T_MODE 0x2A
#define MFRC522_REG_T_PRESCALER 0x2B
#define MFRC522_REG_T_RELOAD_L 0x2C
#define MFRC522_REG_T_RELOAD_H 0x2D

// --- RC522 Commands ---
#define PCD_IDLE 0x00
#define PCD_CALCCRC 0x03
#define PCD_TRANSMIT 0x04
#define PCD_RECEIVE 0x08
#define PCD_TRANSCEIVE 0x0C
#define PCD_RESETPHASE 0x0F

#define PICC_REQIDL 0x26
#define PICC_ANTICOLL 0x93

#define MI_OK 0
#define MI_NOTAGERR 1
#define MI_ERR 2

#include <ctype.h>
// #include <pic18f4520.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <xc.h>
#include "bluetooth.h"
#include "DFPlayer.h"
#include "UART.h"

#define _XTAL_FREQ 4000000

#define STR_MAX 100
#define VR_MAX ((1 << 10) - 1)

// Bluetooth/console RX state
static int volume = 20; // DFPlayer volume 0-30

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
    // printf("BT CMD: %s\n", str);

    __delay_ms(30);

    if (strcmp(str, "FORWARD") == 0)
    {
        // move robot forward
    }
    else if (strcmp(str, "REVERSE") == 0)
    {
        // move robot backward
    }
    else if (strcmp(str, "STRAIGHT") == 0)
    {
    }
    else if (strcmp(str, "TURN_LEFT") == 0)
    {
        // steering left
    }
    else if (strcmp(str, "TURN_RIGHT") == 0)
    {
        // steering right
    }
    else if (strcmp(str, "HIGH_SPEED") == 0)
    {
        // steering left
    }
    else if (strcmp(str, "LOW_SPEED") == 0)
    {
        // steering left
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
        DF_Volume(&volume, 5);
    }
    else if (strcmp(str, "VOL_DOWN") == 0)
    {
        DF_Volume(&volume, -5);
    }
    else
    {
        printf("Unknown CMD: %s\n", str);
    }
    __delay_ms(50);
}

void main(void)
{
    OSCCON = 0x60; // 4MHz
    ADCON1 = 0x0F;
    Initialize_UART();
    DF_Init();
    SPI_Init();
    MFRC522_Init();
    __delay_ms(500);
    unsigned char ver = MFRC522_ReadReg(0x37); // ?????
    // ==========================================
    // [???? 2]??????? (??????)
    // ==========================================
    // RFCfgReg (0x26) -> ?? RxGain ??? 48dB (0x07 << 4)
    MFRC522_WriteReg(0x26, 0x7F);
    // printf("Antenna Gain set to Max.\r\n");

    // ????
    unsigned char temp = MFRC522_ReadReg(MFRC522_REG_TX_CONTROL);
    if (!(temp & 0x03))
    {
        SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
    }
    // printf("Waiting for Card...\r\n");

    unsigned char status;
    unsigned char str[16]; // ???????
    unsigned int fail_count = 0;

    while (1)
    {
        char input_str[STR_MAX];
        if (GetString(input_str))
            keyboard_input(input_str);
        // ?? REQALL (0x52) ???????? REQIDL (0x26) ??
        // 0x52 = ????????? (?????)
        // printf(".");
        unsigned char tx_status = MFRC522_ReadReg(MFRC522_REG_TX_CONTROL);
        if ((tx_status & 0x03) == 0)
        {
            //            printf("[Warning] Antenna was reset! (Power Issue)\r\n");
            // putch('.');
            //  ??????
            SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
        }
        status = MFRC522_Request(0x52, str);

        if (status == MI_OK)
        {
            fail_count = 0;
            printf("Found Card! Reading UID...\r\n"); // ??? 1???????

            status = MFRC522_Anticoll(str);

            if (status == MI_OK)
            {
                printf("UID: %02X %02X %02X %02X \r\n", str[0], str[1], str[2], str[3]);
                if (str[0] == 0xB3 && str[1] == 0x31 && str[2] == 0x4E && str[3] == 0x05)
                {
                    printf("Student Card Detected! Playing Music...\r\n");
                    DF_PlayTrack1();
                }
                else
                {
                }
                // __delay_ms(1000);
            }
            else
            {
                // printf("Error: Collision Fail\r\n"); // ??? 2???? ID
            }
        }
        else
        {
            //            fail_count++;
            //            if ((fail_count % 800) == 0)
            //            { // every 200 misses, dump debug and re-init RF
            //                MFRC522_DebugStatus();
            // MFRC522_Init();
            //            }
            // putch('e');
        }
    }
}
