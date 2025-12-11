#include <xc.h>
#include "SPI.h"
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
void Hard_UART_TX(unsigned char data)
{
  while (!TXSTAbits.TRMT)
    ;           // ç­å¾ä¸ä¸åå­å³å® (Buffer Empty)
  TXREG = data; // ä¸å¥ç¡¬é«å³éæ«å­å¨
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
unsigned char SPI_Transfer(unsigned char data)
{
  SSPBUF = data; // ?????
  while (!SSPSTATbits.BF)
    ;            // ??????
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