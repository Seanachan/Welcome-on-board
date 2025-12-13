#ifndef SPI__H
#define SPI__H

#define PN532_SCK_PIN   LATCbits.LATC3  // Clock
#define PN532_MISO_PIN  PORTCbits.RC4   // Input (從 PN532 讀)
#define PN532_MOSI_PIN  LATCbits.LATC5  // Output (寫給 PN532)
#define PN532_CS_PIN    LATCbits.LATC2  // Chip Select (原本 RC522 是 RA0，這裡改 RC2 方便)

// 腳位方向控制
#define PN532_SCK_TRIS  TRISCbits.TRISC3
#define PN532_MISO_TRIS TRISCbits.TRISC4
#define PN532_MOSI_TRIS TRISCbits.TRISC5
#define PN532_CS_TRIS   TRISCbits.TRISC2

// PN532 指令集
#define PN532_COMMAND_SAMCONFIGURATION     0x14
#define PN532_COMMAND_INLISTPASSIVETARGET  0x4A

#define PN532_SPI_STATREAD  0x02
#define PN532_SPI_DATAWRITE 0x01
#define PN532_SPI_DATAREAD  0x03
#define PN532_SPI_READY     0x01

void SPI_Init(void);
void SPI_WriteByte(unsigned char data);
unsigned char SPI_ReadByte(void);
unsigned char PN532_IsReady(void);
unsigned char PN532_WaitReady(void);
void PN532_SendCommand(unsigned char *cmd, unsigned char len);
void PN532_ReadResponse(unsigned char *buf, unsigned char len);
unsigned char PN532_Init(void);
unsigned char PN532_ReadUID(unsigned char *uid, unsigned char *uidLen);
#endif // SPI__H