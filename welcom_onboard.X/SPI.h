#ifndef SPI__H
#define SPI__H
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