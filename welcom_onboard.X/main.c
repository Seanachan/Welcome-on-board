// ==========================================
//  PIC18F4520 CONFIGURATION BITS
// ==========================================

// CONFIG1H
#pragma config OSC = INTIO67 // Internal Oscillator, I/O on RA6/RA7
#pragma config FCMEN = OFF   // Fail-Safe Clock Monitor Disabled
#pragma config IESO = ON     // Oscillator Switchover Enabled

// CONFIG2L
#pragma config PWRT = OFF      // Power-up Timer Disabled
#pragma config BOREN = SBORDIS // Brown-out Reset Enabled in hardware only
#pragma config BORV = 3        // Brown-out Voltage Minimum

// CONFIG2H
#pragma config WDT = OFF // Watchdog Timer Disabled
#pragma config WDTPS = 1 // Watchdog Postscaler 1:1

// CONFIG3H
#pragma config CCP2MX = PORTC // CCP2 input/output multiplexed with RC1
#pragma config PBADEN = OFF   // IMPORTANT: PORTB<4:0> pins are DIGITAL on Reset
#pragma config LPT1OSC = OFF  // Low-Power Timer1 Oscillator Disabled
#pragma config MCLRE = ON     // MCLR Pin Enabled

// CONFIG4L
#pragma config STVREN = ON // Stack Full/Underflow Reset Enabled
#pragma config LVP = OFF   // Single-Supply ICSP Disabled
#pragma config XINST = OFF // Extended Instruction Set Disabled

// CONFIG5L - CONFIG7H (Code Protection - All Off)
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF
#pragma config CPB = OFF, CPD = OFF
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF
#pragma config WRTC = OFF, WRTB = OFF, WRTD = OFF
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF
#pragma config EBTRB = OFF

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

#define _XTAL_FREQ 4000000

#define STR_MAX 100
#define VR_MAX ((1 << 10) - 1)

// Bluetooth/console RX state
static char buffer[STR_MAX];
static int buffer_size = 0;
static bool btn_interr = false;
static unsigned char mode = 1;
static int volume = 20; // DFPlayer volume 0-30

// --- ?????? ---
void UART_Init(void);
void SPI_Init(void);
void MFRC522_Init(void);
void MFRC522_DebugStatus(void);
void MFRC522_WriteReg(unsigned char addr, unsigned char val);
unsigned char MFRC522_ReadReg(unsigned char addr);

// --- 1. UART ??? (???) ---
void UART_Init(void)
{
    // ????: 1200 bps @ 4MHz (BRGH=0, SPBRG=51)
    // ?? 1200 ??????????????? 9600 (SPBRG=25, BRGH=1) ??????
    OSCCONbits.IRCF = 0b110; // 4 MHz
    TRISC = 0;
    LATC = 0;
    INTCONbits.INT0IF = 0; // Clear INT0 flag
    INTCONbits.INT0IE = 1; // Enable INT0 interrupt
    // PIE1bits.ADIE = 1;     // Enable ADC interrupt
    // PIR1bits.ADIF = 0;     // Clear ADC flag
    INTCONbits.PEIE = 1; // Enable peripheral interrupt
    INTCONbits.GIE = 1;  // Enable global interrupt
    RCONbits.IPEN = 1;   // enable Interrupt Priority mode
    INTCONbits.GIEH = 1; // enable high priority interrupt
    INTCONbits.GIEL = 1; // disable low priority interrupt

    TRISCbits.TRISC6 = 1; // RC6(TX) : Transmiter set 1 (output)
    TRISCbits.TRISC7 = 1; // RC7(RX) : Receiver set 1   (input)

    // Setting Baud rate
    // Baud rate = 1200 (Look up table)
    TXSTAbits.SYNC = 0;    // Synchronus or Asynchronus
    BAUDCONbits.BRG16 = 0; // 16 bits or 8 bits
    TXSTAbits.BRGH = 1;    // High Baud Rate Select bit
    SPBRG = 25;            // Control the period

    // Serial enable
    RCSTAbits.SPEN = 1; // Enable asynchronus serial port (must be set to 1)
    PIR1bits.TXIF = 0;  // Set when TXREG is empty
    PIR1bits.RCIF = 0;  // Will set when reception is complete
    TXSTAbits.TXEN = 1; // Enable transmission
    RCSTAbits.CREN = 1; // Continuous receive enable bit, will be cleared when error occured
    PIE1bits.TXIE = 0;  // Wanna use Interrupt (Transmit)
    IPR1bits.TXIP = 0;  // Interrupt Priority bit
    PIE1bits.RCIE = 1;  // Wanna use Interrupt (Receive)
    IPR1bits.RCIP = 0;  // Interrupt Priority bit
}

// --- 2. SPI ??? (?? PIC18F4520) ---
void SPI_Init(void)
{
    // ?? SPI ????
    TRISCbits.TRISC3 = 0; // SCK (Output)
    TRISCbits.TRISC5 = 0; // SDO (Output)
    TRISCbits.TRISC4 = 1; // SDI (Input)

    // ?? RC522 ????
    MFRC522_CS_TRIS = 0;  // CS Output
    MFRC522_RST_TRIS = 0; // RST Output
    MFRC522_CS_PIN = 1;   // ????? (High)
    MFRC522_RST_PIN = 1;  // RST High (???)

    // ?? SSP ?? (SPI mode 0: CKP=0, CKE=1, sample middle)
    SSPSTAT = 0x40;        // SMP=0, CKE=1
    SSPCON1 = 0x21;        // CKP=0, SPI Master mode, Fosc/16 (faster and stable for RC522)
    SSPCON1bits.SSPEN = 1; // ?? SPI ??
}

// --- 3. SPI ??/???? Byte ---
unsigned char SPI_Transfer(unsigned char data)
{
    SSPBUF = data; // ?????
    while (!SSPSTATbits.BF)
        ;          // ??????
    return SSPBUF; // ???????
}

// --- 4. ?? RC522 ??? ---
void MFRC522_WriteReg(unsigned char addr, unsigned char val)
{
    MFRC522_CS_PIN = 0;               // CS ?????
    SPI_Transfer((addr << 1) & 0x7E); // ???? (??: Address << 1, ????? 0 ???)
    SPI_Transfer(val);                // ????
    MFRC522_CS_PIN = 1;               // CS ?????
}

// --- 5. ?? RC522 ??? ---
unsigned char MFRC522_ReadReg(unsigned char addr)
{
    unsigned char val;
    MFRC522_CS_PIN = 0;                        // CS ??
    SPI_Transfer(((addr << 1) & 0x7E) | 0x80); // ?? (????? 1 ???)
    val = SPI_Transfer(0x00);                  // ?? Dummy Byte ?????
    MFRC522_CS_PIN = 1;                        // CS ??
    return val;
}

//  (Bit Set)
void SetBitMask(unsigned char reg, unsigned char mask)
{
    unsigned char tmp;
    tmp = MFRC522_ReadReg(reg);
    MFRC522_WriteReg(reg, tmp | mask);
}

// (Bit Clear)
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    unsigned char tmp;
    tmp = MFRC522_ReadReg(reg);
    MFRC522_WriteReg(reg, tmp & (~mask));
}

// RC522 power-up and timing setup so transceive commands don't timeout
void MFRC522_Init(void)
{
    MFRC522_WriteReg(MFRC522_REG_COMMAND, PCD_RESETPHASE);
    // __delay_ms(50);
    MFRC522_WriteReg(MFRC522_REG_T_MODE, 0x8D);
    MFRC522_WriteReg(MFRC522_REG_T_PRESCALER, 0x3E);
    MFRC522_WriteReg(MFRC522_REG_T_RELOAD_H, 0x00);
    MFRC522_WriteReg(MFRC522_REG_T_RELOAD_L, 30);
    MFRC522_WriteReg(MFRC522_REG_TX_ASK, 0x40);
    MFRC522_WriteReg(MFRC522_REG_MODE, 0x3D);
    SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}
// quick dump to see why request failed
void MFRC522_DebugStatus(void)
{
    unsigned char err = MFRC522_ReadReg(MFRC522_REG_ERROR);
    unsigned char irq = MFRC522_ReadReg(MFRC522_REG_COMM_IRQ);
    unsigned char stat1 = MFRC522_ReadReg(MFRC522_REG_STATUS1);
    unsigned char stat2 = MFRC522_ReadReg(MFRC522_REG_STATUS2);
    unsigned char fifo = MFRC522_ReadReg(MFRC522_REG_FIFO_LEVEL);
    unsigned char ctrl = MFRC522_ReadReg(MFRC522_REG_CONTROL);
    unsigned char bf = MFRC522_ReadReg(MFRC522_REG_BIT_FRAMING);
    // printf("DBG ERR=%02X IRQ=%02X S1=%02X S2=%02X FIFO=%02X CTRL=%02X BF=%02X\r\n", err, irq, stat1, stat2, fifo, ctrl, bf);
}

// data change with card
unsigned char MFRC522_ToCard(unsigned char command, unsigned char *sendData, unsigned char sendLen, unsigned char *backData, unsigned int *backLen)
{
    unsigned char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitIRq = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;

    switch (command)
    {
    case PCD_TRANSCEIVE:
        irqEn = 0x77;
        waitIRq = 0x30;
        break;
    }

    MFRC522_WriteReg(MFRC522_REG_COMM_IEN, irqEn | 0x80);
    ClearBitMask(MFRC522_REG_COMM_IRQ, 0x80);
    SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80); // FlushBuffer

    MFRC522_WriteReg(MFRC522_REG_COMMAND, PCD_IDLE);

    // write data in FIFO
    for (i = 0; i < sendLen; i++)
    {
        MFRC522_WriteReg(MFRC522_REG_FIFO_DATA, sendData[i]);
    }

    // execute command
    MFRC522_WriteReg(MFRC522_REG_COMMAND, command);
    if (command == PCD_TRANSCEIVE)
    {
        SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80); // StartSend
    }

    // polling
    i = 25000;
    do
    {
        n = MFRC522_ReadReg(MFRC522_REG_COMM_IRQ);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);

    if (i != 0)
    {
        if (!(MFRC522_ReadReg(MFRC522_REG_ERROR) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }

            if (command == PCD_TRANSCEIVE)
            {
                n = MFRC522_ReadReg(MFRC522_REG_FIFO_LEVEL);
                lastBits = MFRC522_ReadReg(MFRC522_REG_CONTROL) & 0x07;
                if (lastBits)
                {
                    *backLen = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *backLen = n * 8;
                }

                if (n == 0)
                {
                    n = 1;
                }

                // read the response in FLFO
                for (i = 0; i < n; i++)
                {
                    backData[i] = MFRC522_ReadReg(MFRC522_REG_FIFO_DATA);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }

    return status;
}

// check if the card exist
unsigned char MFRC522_Request(unsigned char reqMode, unsigned char *TagType)
{
    unsigned char status;
    unsigned int backBits;

    MFRC522_WriteReg(MFRC522_REG_BIT_FRAMING, 0x07); // TxLastBists = BitFramingReg[2..0]

    TagType[0] = reqMode;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

    if ((status != MI_OK) || (backBits != 0x10))
    {
        status = MI_ERR;
    }
    return status;
}

// 2.
unsigned char MFRC522_Anticoll(unsigned char *serNum)
{
    unsigned char status;
    unsigned char i;
    unsigned char serNumCheck = 0;
    unsigned int unLen;

    ClearBitMask(MFRC522_REG_STATUS2, 0x08); // TempSensconf
    ClearBitMask(MFRC522_REG_COLL, 0x80);    // ValuesAfterColl
    MFRC522_WriteReg(MFRC522_REG_BIT_FRAMING, 0x00);

    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK)
    {
        // ?? Checksum
        for (i = 0; i < 4; i++)
        {
            serNumCheck ^= serNum[i];
        }
        if (serNumCheck != serNum[4])
        {
            status = MI_ERR;
        }
    }
    return status;
}

void Hard_UART_TX(unsigned char data)
{
    while (!TXSTAbits.TRMT)
        ;         // ç­å¾ä¸ä¸åå­å³å® (Buffer Empty)
    TXREG = data; // ä¸å¥ç¡¬é«å³éæ«å­å¨
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

    // process other interrupt sources here, if required
    return;
}

void main(void)
{
    OSCCON = 0x60; // 4MHz
    ADCON1 = 0x0F;
    UART_Init();
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

    // DF_PlayTrack1();

    while (1)
    {
        char input_str[STR_MAX];
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

        if (GetString(input_str))
            keyboard_input(input_str);
    }
}
