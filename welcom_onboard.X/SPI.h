#ifndef SPI__H
#define SPI__H
// --- RC522 RFID Pins ---
#define MFRC522_CS_PIN LATAbits.LATA0 // Chip Select (SDA)
#define MFRC522_CS_TRIS TRISAbits.TRISA0

#define MFRC522_RST_PIN LATEbits.LATE0 // Reset (RST)
#define MFRC522_RST_TRIS TRISEbits.TRISE0

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
void SPI_Init(void);
void MFRC522_Init(void);
unsigned char SPI_Transfer(unsigned char data);
void MFRC522_WriteReg(unsigned char addr, unsigned char val);
unsigned char MFRC522_ReadReg(unsigned char addr);
void SetBitMask(unsigned char reg, unsigned char mask);
void ClearBitMask(unsigned char reg, unsigned char mask);
unsigned char MFRC522_ToCard(unsigned char command, unsigned char *sendData, unsigned char sendLen, unsigned char *backData, unsigned int *backLen);
unsigned char MFRC522_Request(unsigned char reqMode, unsigned char *TagType);
unsigned char MFRC522_Anticoll(unsigned char *serNum);

#endif // SPI__H