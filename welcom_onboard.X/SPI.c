#include <xc.h>
#include "SPI.h"
#include "UART.h"
#include<stdio.h>
void SPI_Init(void) {
    PN532_CS_TRIS = 0;   // Output
    PN532_SCK_TRIS = 0;  // Output
    PN532_MOSI_TRIS = 0; // Output
    PN532_MISO_TRIS = 1; // Input

    PN532_CS_PIN = 1;    // Idle High
    PN532_SCK_PIN = 0;   // Idle Low
}

// LSB First å¯«å¥
void SPI_WriteByte(unsigned char data) {
    for(int i=0; i<8; i++) {
        if(data & 0x01) PN532_MOSI_PIN = 1;
        else            PN532_MOSI_PIN = 0;
        data >>= 1;

        PN532_SCK_PIN = 1; // Clock High
        __delay_us(50); 
        PN532_SCK_PIN = 0; // Clock Low
        __delay_us(50);
    }
}

// LSB First è®å
unsigned char SPI_ReadByte(void) {
    unsigned char data = 0;
    for(int i=0; i<8; i++) {
        PN532_SCK_PIN = 1; // Clock High (PN532 shifts out)
        __delay_us(2);

        if(PN532_MISO_PIN) data |= (1 << i);

        PN532_SCK_PIN = 0; // Clock Low
        __delay_us(2);
    }
    return data;
}

// æª¢æ¥ PN532 æ¯å¦æºåå¥½ (Status Read)
unsigned char PN532_IsReady(void) {
    unsigned char status;
    PN532_CS_PIN = 0;
    __delay_us(10);
    SPI_WriteByte(PN532_SPI_STATREAD); // 0x02
    status = SPI_ReadByte();
    PN532_CS_PIN = 1;
//    printf("%02X\r\n",status);
    return (status == PN532_SPI_READY);
}

// ç­å¾ç´å° Ready
unsigned char PN532_WaitReady(void) {
    unsigned int timeout = 1000; // 2 sec
    while(timeout > 0) {
        if(PN532_IsReady()) return 1;
        __delay_ms(1);
        timeout--;
    }
    return 0;
}

// ç¼éæä»¤å°å
void PN532_SendCommand(unsigned char *cmd, unsigned char len) {
    unsigned char checksum = 0;
    
    PN532_CS_PIN = 0;
    __delay_us(10);
    
    SPI_WriteByte(PN532_SPI_DATAWRITE); // 0x01
    
    SPI_WriteByte(0x00); // Preamble
    SPI_WriteByte(0x00); // Start Code 1
    SPI_WriteByte(0xFF); // Start Code 2
    
    SPI_WriteByte(len + 1);
    SPI_WriteByte(~(len + 1) + 1);
    
    SPI_WriteByte(0xD4); // Host to PN532
    checksum += 0xD4;
    
    for(int i=0; i<len; i++) {
        SPI_WriteByte(cmd[i]);
        checksum += cmd[i];
    }
    
    SPI_WriteByte(~checksum + 1);
    SPI_WriteByte(0x00); // Postamble
    
    PN532_CS_PIN = 1;
}

// è®ååæå°å
void PN532_ReadResponse(unsigned char *buf, unsigned char len) {
    PN532_CS_PIN = 0;
    __delay_us(10);
    SPI_WriteByte(PN532_SPI_DATAREAD); // 0x03
    
    for(int i=0; i<len; i++) {
        buf[i] = SPI_ReadByte();
    }
    PN532_CS_PIN = 1;
}

// åå§å PN532
unsigned char PN532_Init(void) {
    unsigned char cmd[] = {PN532_COMMAND_SAMCONFIGURATION, 0x01, 0x14, 0x01};
    unsigned char dummy[20];

//    printf("Init PN532 (SPI)...\r\n");
    
    PN532_SendCommand(cmd, sizeof(cmd));
    
    if(!PN532_WaitReady()) {
//        printf("Error: PN532 Not Ready. Check Wiring & DIP Switch!\r\n");
        return 0;
    }
    
    
    PN532_ReadResponse(dummy, 15); 
     //    printf("PN532 Ready!\r\n");
    return 1;
}

// è®å UID
// ===================== ä¿®æ­£å¾çè®å¡å½å¼ =====================

// è®å UID (ä¿®æ­£çï¼èç ACK + Data åæ®µ)
unsigned char PN532_ReadUID(unsigned char *uid, unsigned char *uidLen) {
    unsigned char cmd[] = {PN532_COMMAND_INLISTPASSIVETARGET, 0x01, 0x00};
    unsigned char ackBuffer[10];
    unsigned char buf[32];
    
    // 1. ç¼éãè®å¡ãæä»¤
    PN532_SendCommand(cmd, sizeof(cmd));
    
    // 2. ç­å¾ PN532 æ¶å°æä»¤ (ç¬¬ä¸æ¬¡ Ready)
    if(!PN532_WaitReady()) return 0; 
    
    // 3. ãééµä¿®æ­£ãåè®å ACK (ç¢ºèªå®æ¶å°æä»¤äº)
    // PN532 ç ACK åºå®æ¯ 6 bytes: 00 00 FF 00 FF 00
    PN532_ReadResponse(ackBuffer, 6); 
    
    
    if(!PN532_WaitReady()) return 0; // å¦æéè£¡è¶æï¼ä»£è¡¨æ²å¡çæææçµæ
    
    // 5. è®åçæ­£çè³æ (UID å¨éè£¡é¢)
    PN532_ReadResponse(buf, 26);
    
    
    int offset = 0;
    while(offset < 20 && buf[offset] != 0xD5) offset++;
    
    if(offset >= 20) return 0; // æ²æ¾å°æ¨é ­
    
    if(buf[offset+2] != 1) return 0; 
    
    *uidLen = buf[offset+6]; // NFCID Length
    
    for(int i=0; i<*uidLen; i++) {
        uid[i] = buf[offset+7+i];
    }
    
    return 1;
}