#include <xc.h>
#include "SPI.h"
#include "UART.h"
void SPI_Init(void) {
    PN532_CS_TRIS = 0;   // Output
    PN532_SCK_TRIS = 0;  // Output
    PN532_MOSI_TRIS = 0; // Output
    PN532_MISO_TRIS = 1; // Input

    PN532_CS_PIN = 1;    // Idle High
    PN532_SCK_PIN = 0;   // Idle Low
}

// LSB First 寫入
void SPI_WriteByte(unsigned char data) {
    for(int i=0; i<8; i++) {
        if(data & 0x01) PN532_MOSI_PIN = 1;
        else            PN532_MOSI_PIN = 0;
        data >>= 1;

        PN532_SCK_PIN = 1; // Clock High
        __delay_us(2); 
        PN532_SCK_PIN = 0; // Clock Low
        __delay_us(2);
    }
}

// LSB First 讀取
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

// 檢查 PN532 是否準備好 (Status Read)
unsigned char PN532_IsReady(void) {
    unsigned char status;
    PN532_CS_PIN = 0;
    __delay_us(10);
    SPI_WriteByte(PN532_SPI_STATREAD); // 0x02
    status = SPI_ReadByte();
    PN532_CS_PIN = 1;
    return (status == PN532_SPI_READY);
}

// 等待直到 Ready
unsigned char PN532_WaitReady(void) {
    unsigned int timeout = 1000; // 2 sec
    while(timeout > 0) {
        if(PN532_IsReady()) return 1;
        __delay_ms(1);
        timeout--;
    }
    return 0;
}

// 發送指令封包
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

// 讀取回應封包
void PN532_ReadResponse(unsigned char *buf, unsigned char len) {
    PN532_CS_PIN = 0;
    __delay_us(10);
    SPI_WriteByte(PN532_SPI_DATAREAD); // 0x03
    
    for(int i=0; i<len; i++) {
        buf[i] = SPI_ReadByte();
    }
    PN532_CS_PIN = 1;
}

// 初始化 PN532
unsigned char PN532_Init(void) {
    unsigned char cmd[] = {PN532_COMMAND_SAMCONFIGURATION, 0x01, 0x14, 0x01};
    unsigned char dummy[20];

//    printf("Init PN532 (SPI)...\r\n");
    
    PN532_SendCommand(cmd, sizeof(cmd));
    
    if(!PN532_WaitReady()) {
//        printf("Error: PN532 Not Ready. Check Wiring & DIP Switch!\r\n");
        return 0;
    }
    
    // 讀取回應 (ACK + Data)
    PN532_ReadResponse(dummy, 15); 
    
//    printf("PN532 Ready!\r\n");
    return 1;
}

// 讀取 UID
// ===================== 修正後的讀卡函式 =====================

// 讀取 UID (修正版：處理 ACK + Data 分段)
unsigned char PN532_ReadUID(unsigned char *uid, unsigned char *uidLen) {
    unsigned char cmd[] = {PN532_COMMAND_INLISTPASSIVETARGET, 0x01, 0x00};
    unsigned char ackBuffer[10];
    unsigned char buf[32];
    
    // 1. 發送「讀卡」指令
    PN532_SendCommand(cmd, sizeof(cmd));
    
    // 2. 等待 PN532 收到指令 (第一次 Ready)
    if(!PN532_WaitReady()) return 0; 
    
    // 3. 【關鍵修正】先讀取 ACK (確認它收到指令了)
    // PN532 的 ACK 固定是 6 bytes: 00 00 FF 00 FF 00
    PN532_ReadResponse(ackBuffer, 6); 
    
    
    if(!PN532_WaitReady()) return 0; // 如果這裡超時，代表沒卡片或掃描結束
    
    // 5. 讀取真正的資料 (UID 在這裡面)
    PN532_ReadResponse(buf, 26);
    
    
    int offset = 0;
    while(offset < 20 && buf[offset] != 0xD5) offset++;
    
    if(offset >= 20) return 0; // 沒找到標頭
    
    if(buf[offset+2] != 1) return 0; 
    
    *uidLen = buf[offset+6]; // NFCID Length
    
    for(int i=0; i<*uidLen; i++) {
        uid[i] = buf[offset+7+i];
    }
    
    return 1;
}