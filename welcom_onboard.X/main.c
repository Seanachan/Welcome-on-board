// CONFIG1H
#pragma config OSC = INTIO67  // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF    // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = ON      // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF  // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 1  // Watchdog Timer Postscale Select bits (1:1)

// CONFIG3H
#pragma config CCP2MX = PORTC  // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = ON     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF   // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON      // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON  // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF    // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config XINST = OFF  // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF  // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF  // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF  // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF  // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF  // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF  // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF  // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF  // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF  // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF  // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF  // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF  // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF  // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF  // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF  // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF  // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF  // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF  
// --- RC522 register ---
#define MFRC522_REG_COMMAND     0x01
#define MFRC522_REG_COMM_IEN    0x02
#define MFRC522_REG_DIV_IEN     0x03
#define MFRC522_REG_COMM_IRQ    0x04
#define MFRC522_REG_DIV_IRQ     0x05
#define MFRC522_REG_ERROR       0x06
#define MFRC522_REG_STATUS1     0x07
#define MFRC522_REG_STATUS2     0x08
#define MFRC522_REG_FIFO_DATA   0x09
#define MFRC522_REG_FIFO_LEVEL  0x0A
#define MFRC522_REG_CONTROL     0x0C
#define MFRC522_REG_BIT_FRAMING 0x0D
#define MFRC522_REG_COLL        0x0E
#define MFRC522_REG_MODE        0x11
#define MFRC522_REG_TX_ASK      0x15
#define MFRC522_REG_TX_CONTROL  0x14
#define MFRC522_REG_T_MODE      0x2A
#define MFRC522_REG_T_PRESCALER 0x2B
#define MFRC522_REG_T_RELOAD_L  0x2C
#define MFRC522_REG_T_RELOAD_H  0x2D

// --- RC522 command ---
#define PCD_IDLE              0x00
#define PCD_CALCCRC           0x03
#define PCD_TRANSMIT          0x04
#define PCD_RECEIVE           0x08
#define PCD_TRANSCEIVE        0x0C
#define PCD_RESETPHASE        0x0F

#define PICC_REQIDL           0x26
#define PICC_ANTICOLL         0x93

#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2

#define MFRC522_CS_PIN  LATAbits.LATA0   // SDA(SS)
#define MFRC522_CS_TRIS TRISAbits.TRISA0 

#define MFRC522_RST_PIN  LATEbits.LATE0  // RST
#define MFRC522_RST_TRIS TRISEbits.TRISE0

// ?? MP3 ???? (? DFPlayer ? RX)
#define MP3_TX_PIN LATBbits.LATB1
#define MP3_TX_TRIS TRISBbits.TRISB1

#include <ctype.h>
#include <pic18f4520.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <xc.h>

#define _XTAL_FREQ 4000000

#define STR_MAX 100
#define VR_MAX ((1 << 10) - 1)

// Bluetooth/console RX state
static char buffer[STR_MAX];
static int buffer_size = 0;
static bool btn_interr = false;
static unsigned char mode = 1;
static int volume = 20; // DFPlayer volume 0-30

// Software UART on RB4 (DFPlayer fallback)
#define SW_TX_PIN LATBbits.LATB4
#define SW_TX_TRIS TRISBbits.TRISB4

// --- ?????? ---
void UART_Init(void);
void SPI_Init(void);
void putch(char data);
void MFRC522_Init(void);
void MFRC522_DebugStatus(void);
void MFRC522_WriteReg(unsigned char addr, unsigned char val);
unsigned char MFRC522_ReadReg(unsigned char addr);
void DF_SelectTF(void);
void DF_SetVolume(unsigned char level);
void DF_PlayRootTrack(unsigned int trackNum);
void SoftUART_Init(void);
void SoftUART_Write(unsigned char data);
void Send_DFPlayer_Command(unsigned char command, unsigned int parameter);
void mp3_Init(void);
void mp3_Play(int track);
void mp3_Stop(void);

// --- Software UART (RB4) from software_UART.c ---
void SoftUART_Init(void) {
    SW_TX_TRIS = 0; // TX as output
    SW_TX_PIN = 1;  // idle high
}

void SoftUART_Write(unsigned char data) {
    unsigned char interrupt_status = INTCONbits.GIE;
    INTCONbits.GIE = 0; // hold timing while sending

    SW_TX_PIN = 0;       // start bit
    __delay_us(104);     // 9600 baud @4MHz

    for (int i = 0; i < 8; i++) {
        SW_TX_PIN = (data & 1) ? 1 : 0;
        data >>= 1;
        __delay_us(104);
    }

    SW_TX_PIN = 1; // stop bit
    __delay_us(104);

    INTCONbits.GIE = interrupt_status;
}

void Send_DFPlayer_Command(unsigned char command, unsigned int parameter) {
    unsigned char cmd_data[6] = {0xFF, 0x06, command, 0x00,
                                 (unsigned char)(parameter >> 8),
                                 (unsigned char)(parameter & 0xFF)};

    unsigned int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += cmd_data[i];
    }
    unsigned int checksum = 0xFFFF - sum + 1;

    SoftUART_Write(0x7E);
    for (int i = 0; i < 6; i++) {
        SoftUART_Write(cmd_data[i]);
    }
    SoftUART_Write((unsigned char)(checksum >> 8));
    SoftUART_Write((unsigned char)(checksum & 0xFF));
    SoftUART_Write(0xEF);

    __delay_ms(50);
}

void mp3_Init(void) {
    SoftUART_Init();
    __delay_ms(1000); // wait for DFPlayer to power up
    Send_DFPlayer_Command(0x06, 25); // default volume
}

void mp3_Play(int track) {
    Send_DFPlayer_Command(0x03, track);
}

void mp3_Stop(void) {
    Send_DFPlayer_Command(0x16, 0);
}

// --- 1. UART ??? (???) ---
void UART_Init(void) {
    // ????: 1200 bps @ 4MHz (BRGH=0, SPBRG=51)
    // ?? 1200 ??????????????? 9600 (SPBRG=25, BRGH=1) ??????
    TXSTAbits.SYNC = 0;     
    BAUDCONbits.BRG16 = 0;  
    TXSTAbits.BRGH = 1;     
    SPBRG = 25;             

    TRISCbits.TRISC6 =0;   // TX ?? Output (????????????0????)
    TRISCbits.TRISC7 = 1;   // RX ?? Input
    
    // [??????] ??????
    RCSTAbits.SPEN = 1;     // ?? Serial Port (??!)
    TXSTAbits.TXEN = 1;     // ???? (??!)
    RCSTAbits.CREN = 1;     // ???? (??????????)
}

// --- 2. SPI ??? (?? PIC18F4520) ---
void SPI_Init(void) {
    // ?? SPI ????
    TRISCbits.TRISC3 = 0;   // SCK (Output)
    TRISCbits.TRISC5 = 0;   // SDO (Output)
    TRISCbits.TRISC4 = 1;   // SDI (Input)

    // ?? RC522 ????
    MFRC522_CS_TRIS = 0;    // CS Output
    MFRC522_RST_TRIS = 0;   // RST Output
    MFRC522_CS_PIN = 1;     // ????? (High)
    MFRC522_RST_PIN = 1;    // RST High (???)

    // ?? SSP ?? (SPI mode 0: CKP=0, CKE=1, sample middle)
    SSPSTAT = 0x40;         // SMP=0, CKE=1
    SSPCON1 = 0x21;         // CKP=0, SPI Master mode, Fosc/16 (faster and stable for RC522)
    SSPCON1bits.SSPEN = 1;  // ?? SPI ??
}

// --- 3. SPI ??/???? Byte ---
unsigned char SPI_Transfer(unsigned char data) {
    SSPBUF = data;           // ?????
    while(!SSPSTATbits.BF);  // ??????
    return SSPBUF;           // ???????
}

// --- 4. ?? RC522 ??? ---
void MFRC522_WriteReg(unsigned char addr, unsigned char val) {
    MFRC522_CS_PIN = 0;             // CS ?????
    SPI_Transfer((addr << 1) & 0x7E); // ???? (??: Address << 1, ????? 0 ???)
    SPI_Transfer(val);              // ????
    MFRC522_CS_PIN = 1;             // CS ?????
}

// --- 5. ?? RC522 ??? ---
unsigned char MFRC522_ReadReg(unsigned char addr) {
    unsigned char val;
    MFRC522_CS_PIN = 0;             // CS ??
    SPI_Transfer(((addr << 1) & 0x7E) | 0x80); // ?? (????? 1 ???)
    val = SPI_Transfer(0x00);       // ?? Dummy Byte ?????
    MFRC522_CS_PIN = 1;             // CS ??
    return val;
}

void putch(char data) {
    if (data == '\n' || data == '\r') {
        while(!TXSTAbits.TRMT);
        TXREG = '\r';
        while(!TXSTAbits.TRMT);
        TXREG = '\n';
    } else {
        while(!TXSTAbits.TRMT);
        TXREG = data;
    }
}
//  (Bit Set)
void SetBitMask(unsigned char reg, unsigned char mask) {
    unsigned char tmp;
    tmp = MFRC522_ReadReg(reg);
    MFRC522_WriteReg(reg, tmp | mask);
}

// (Bit Clear)
void ClearBitMask(unsigned char reg, unsigned char mask) {
    unsigned char tmp;
    tmp = MFRC522_ReadReg(reg);
    MFRC522_WriteReg(reg, tmp & (~mask));
}

// RC522 power-up and timing setup so transceive commands don't timeout
void MFRC522_Init(void) {
    MFRC522_WriteReg(MFRC522_REG_COMMAND, PCD_RESETPHASE);
    __delay_ms(50);
    MFRC522_WriteReg(MFRC522_REG_T_MODE, 0x8D);
    MFRC522_WriteReg(MFRC522_REG_T_PRESCALER, 0x3E);
    MFRC522_WriteReg(MFRC522_REG_T_RELOAD_H, 0x00);
    MFRC522_WriteReg(MFRC522_REG_T_RELOAD_L, 30);
    MFRC522_WriteReg(MFRC522_REG_TX_ASK, 0x40);
    MFRC522_WriteReg(MFRC522_REG_MODE, 0x3D);
    SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}
// quick dump to see why request failed
void MFRC522_DebugStatus(void) {
    unsigned char err = MFRC522_ReadReg(MFRC522_REG_ERROR);
    unsigned char irq = MFRC522_ReadReg(MFRC522_REG_COMM_IRQ);
    unsigned char stat1 = MFRC522_ReadReg(MFRC522_REG_STATUS1);
    unsigned char stat2 = MFRC522_ReadReg(MFRC522_REG_STATUS2);
    unsigned char fifo = MFRC522_ReadReg(MFRC522_REG_FIFO_LEVEL);
    unsigned char ctrl = MFRC522_ReadReg(MFRC522_REG_CONTROL);
    unsigned char bf = MFRC522_ReadReg(MFRC522_REG_BIT_FRAMING);
    //printf("DBG ERR=%02X IRQ=%02X S1=%02X S2=%02X FIFO=%02X CTRL=%02X BF=%02X\r\n", err, irq, stat1, stat2, fifo, ctrl, bf);
}

//data change with card
unsigned char MFRC522_ToCard(unsigned char command, unsigned char *sendData, unsigned char sendLen, unsigned char *backData, unsigned int *backLen) {
    unsigned char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitIRq = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;

    switch (command) {
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
    for (i = 0; i < sendLen; i++) {
        MFRC522_WriteReg(MFRC522_REG_FIFO_DATA, sendData[i]);
    }

    // execute command
    MFRC522_WriteReg(MFRC522_REG_COMMAND, command);
    if (command == PCD_TRANSCEIVE) {
        SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80); // StartSend
    }

    // polling
    i = 25000;
    do {
        n = MFRC522_ReadReg(MFRC522_REG_COMM_IRQ);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);

    if (i != 0) {
        if (!(MFRC522_ReadReg(MFRC522_REG_ERROR) & 0x1B)) {
            status = MI_OK;
            if (n & irqEn & 0x01) {
                status = MI_NOTAGERR;
            }

            if (command == PCD_TRANSCEIVE) {
                n = MFRC522_ReadReg(MFRC522_REG_FIFO_LEVEL);
                lastBits = MFRC522_ReadReg(MFRC522_REG_CONTROL) & 0x07;
                if (lastBits) {
                    *backLen = (n - 1) * 8 + lastBits;
                } else {
                    *backLen = n * 8;
                }

                if (n == 0) {
                    n = 1;
                }

                // read the response in FLFO
                for (i = 0; i < n; i++) {
                    backData[i] = MFRC522_ReadReg(MFRC522_REG_FIFO_DATA);
                }
            }
        } else {
            status = MI_ERR;
        }
    }

    return status;
}

// check if the card exist
unsigned char MFRC522_Request(unsigned char reqMode, unsigned char *TagType) {
    unsigned char status;
    unsigned int backBits;

    MFRC522_WriteReg(MFRC522_REG_BIT_FRAMING, 0x07); // TxLastBists = BitFramingReg[2..0]
    
    TagType[0] = reqMode;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

    if ((status != MI_OK) || (backBits != 0x10)) {
        status = MI_ERR;
    }
    return status;
}

// 2.
unsigned char MFRC522_Anticoll(unsigned char *serNum) {
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

    if (status == MI_OK) {
        // ?? Checksum
        for (i = 0; i < 4; i++) {
            serNumCheck ^= serNum[i];
        }
        if (serNumCheck != serNum[4]) {
            status = MI_ERR;
        }
    }
    return status;
}

void Hard_UART_TX(unsigned char data) {
    while(!TXSTAbits.TRMT); // ç­å¾ä¸ä¸åå­å³å® (Buffer Empty)
    TXREG = data;           // ä¸å¥ç¡¬é«å³éæ«å­å¨
}

// --- ??????? DFPlayer ---
void DF_SendCmd(unsigned char cmd, unsigned char param1, unsigned char param2) {
    unsigned int checksum;
    unsigned char cmd_buf[10];

    cmd_buf[0] = 0x7E; // Start Byte
    cmd_buf[1] = 0xFF; // Version
    cmd_buf[2] = 0x06; // Length
    cmd_buf[3] = cmd;  // Command
    cmd_buf[4] = 0x00; // Feedback (0=No feedback)
    cmd_buf[5] = param1; // Parameter High
    cmd_buf[6] = param2; // Parameter Low
    
    // ?? Checksum (0 - (Sum of bytes))
    checksum = 0 - (cmd_buf[1] + cmd_buf[2] + cmd_buf[3] + cmd_buf[4] + cmd_buf[5] + cmd_buf[6]);
    cmd_buf[7] = (unsigned char)(checksum >> 8); // Checksum High
    cmd_buf[8] = (unsigned char)(checksum);      // Checksum Low
    cmd_buf[9] = 0xEF; // End Byte

    // ???? UART ??? 10 ? Bytes
    for (int i = 0; i < 10; i++) {
        Hard_UART_TX(cmd_buf[i]);
    }
}

// --- DFPlayer helpers ---
void DF_SelectTF(void) {
    DF_SendCmd(0x09, 0x00, 0x02); // select TF card as source
}

void DF_SetVolume(unsigned char level) {
    if (level > 30) level = 30;
    DF_SendCmd(0x06, 0x00, level);
}

void DF_PlayRootTrack(unsigned int trackNum) {
    // 0x03 plays file in root named 0001.mp3 -> trackNum=1, up to 2999
    DF_SendCmd(0x03, (trackNum >> 8) & 0xFF, trackNum & 0xFF);
}
// --- ??? ---
void main(void) {
    OSCCON = 0x60; // 4MHz
    ADCON1 = 0x0F;
    UART_Init();
    SPI_Init();
    MFRC522_Init();
    //MP3_TX_TRIS = 0; // ????
    //MP3_TX_PIN = 1;  // ????? High
    __delay_ms(5000);
    DF_SelectTF();
    DF_SetVolume(20);
    unsigned char ver = MFRC522_ReadReg(0x37); // ?????
    //printf("Check Version: 0x%02X ", ver);

    if (ver == 0x92 || ver == 0x91 || ver == 0x88) {
        //printf("-> (OK! SPI Works)\r\n");
    } else {
        //printf("-> (Error! SPI Fail)\r\n");
        //printf("System Halted.\r\n");
        while(1); // ???????????????????
    }

    // ==========================================
    // [???? 2]??????? (??????)
    // ==========================================
    // RFCfgReg (0x26) -> ?? RxGain ??? 48dB (0x07 << 4)
    MFRC522_WriteReg(0x26, 0x7F); 
    //printf("Antenna Gain set to Max.\r\n");

    // ????
    unsigned char temp = MFRC522_ReadReg(MFRC522_REG_TX_CONTROL); 
    if (!(temp & 0x03)) {
        SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
    }
    //printf("Waiting for Card...\r\n");

    
    unsigned char status;
    unsigned char str[16]; // ???????
    unsigned int fail_count = 0;

    while(1) {
        // ?? REQALL (0x52) ???????? REQIDL (0x26) ??
        // 0x52 = ????????? (?????)
        unsigned char tx_status = MFRC522_ReadReg(MFRC522_REG_TX_CONTROL); 
        if ((tx_status & 0x03) == 0) {
            //printf("[Warning] Antenna was reset! (Power Issue)\r\n");
            // ??????
            SetBitMask(MFRC522_REG_TX_CONTROL, 0x03); 
        }
        status = MFRC522_Request(0x26, str); 
        
        if (status == MI_OK) {
            fail_count = 0;
            //printf("Found Card! Reading UID...\r\n"); // ??? 1???????
            
            status = MFRC522_Anticoll(str);
            
            if (status == MI_OK) {
                //printf("UID: %02X %02X %02X %02X \r\n", str[0], str[1], str[2], str[3]);
               if(str[0] == 0xB3 && str[1] == 0x31 && str[2] == 0x4E && str[3] == 0x05) {
                    
                    //printf("Student Card Detected! Playing Music...\r\n");
                    // play 0001.mp3 from root
                    DF_PlayTrack1();                    
                } else {
                    
                }
                __delay_ms(1000); 
            } else {
                //printf("Error: Collision Fail\r\n"); // ??? 2???? ID
            }
        } 
        else {
            fail_count++;
            if ((fail_count % 200) == 0) { // every 200 misses, dump debug and re-init RF
                MFRC522_DebugStatus();
                MFRC522_Init();
            }
            putch('.'); 
        }
    }
}
