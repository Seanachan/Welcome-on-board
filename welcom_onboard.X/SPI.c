#include <xc.h>
#include "SPI.h"
#include "UART.h"
#include <stdio.h>

/**
 * Initialize SPI GPIO pins for PN532 connection.
 * Configures SCK, MOSI, and CS as outputs, MISO as input.
 */
void SPI_Init(void) {
    PN532_CS_TRIS = 0;   // Set Chip Select pin as Output
    PN532_SCK_TRIS = 0;  // Set Serial Clock pin as Output
    PN532_MOSI_TRIS = 0; // Set Master Out Slave In pin as Output
    PN532_MISO_TRIS = 1; // Set Master In Slave Out pin as Input

    PN532_CS_PIN = 1;    // Pull CS High (Idle state)
    PN532_SCK_PIN = 0;   // Set Clock to Low (CPOL = 0)
}

/**
 * Write one byte to SPI bus using Bit-Banging (LSB First).
 * @param data: The byte to be transmitted.
 */
void SPI_WriteByte(unsigned char data) {
    for(int i=0; i<8; i++) {
        // Prepare data on MOSI pin (LSB First)
        if(data & 0x01) PN532_MOSI_PIN = 1;
        else            PN532_MOSI_PIN = 0;
        data >>= 1;

        // Pulse the clock to trigger data sampling by PN532
        PN532_SCK_PIN = 1; // Clock High
        __delay_us(50); 
        PN532_SCK_PIN = 0; // Clock Low
        __delay_us(50);
    }
}

/**
 * Read one byte from SPI bus using Bit-Banging (LSB First).
 * @return: The byte received from the slave device.
 */
unsigned char SPI_ReadByte(void) {
    unsigned char data = 0;
    for(int i=0; i<8; i++) {
        PN532_SCK_PIN = 1; // Set Clock High (PN532 shifts out data bit)
        __delay_us(2);

        // Sample the MISO pin and store in the corresponding bit position
        if(PN532_MISO_PIN) data |= (1 << i);

        PN532_SCK_PIN = 0; // Set Clock Low
        __delay_us(2);
    }
    return data;
}

/**
 * Check if the PN532 is ready to communicate by reading the status byte.
 * @return: 1 if PN532_SPI_READY (0x01) is received, 0 otherwise.
 */
unsigned char PN532_IsReady(void) {
    unsigned char status;
    PN532_CS_PIN = 0; // Select PN532
    __delay_us(10);
    
    SPI_WriteByte(PN532_SPI_STATREAD); // Send Status Read command (0x02)
    status = SPI_ReadByte();           // Read 1-byte status
    
    PN532_CS_PIN = 1; // Deselect PN532
    return (status == PN532_SPI_READY);
}

/**
 * Wait for the PN532 to become ready within a specific timeout.
 * @return: 1 if ready, 0 if timeout (approx. 2 seconds).
 */
unsigned char PN532_WaitReady(void) {
    unsigned int timeout = 1000; // 1000 iterations * 1ms delay = 1 second
    while(timeout > 0) {
        if(PN532_IsReady()) return 1;
        __delay_ms(1);
        timeout--;
    }
    return 0;
}

/**
 * Construct and send a PN532 information frame over SPI.
 * Frame format: Preamble, StartCodes, Length, Length Checksum, Data, Checksum, Postamble.
 * @param cmd: Pointer to the command data buffer.
 * @param len: Length of the command data.
 */
void PN532_SendCommand(unsigned char *cmd, unsigned char len) {
    unsigned char checksum = 0;
    
    PN532_CS_PIN = 0; // Select PN532
    __delay_us(10);
    
    SPI_WriteByte(PN532_SPI_DATAWRITE); // SPI Header: Data Write (0x01)
    
    // PN532 Frame Structure
    SPI_WriteByte(0x00); // Preamble
    SPI_WriteByte(0x00); // Start Code 1
    SPI_WriteByte(0xFF); // Start Code 2
    
    SPI_WriteByte(len + 1);       // TFI (1 byte) + Command Data length
    SPI_WriteByte(~(len + 1) + 1); // Length Checksum (LCS)
    
    SPI_WriteByte(0xD4); // TFI: Direction from Host to PN532
    checksum += 0xD4;
    
    // Write Command Payload
    for(int i=0; i<len; i++) {
        SPI_WriteByte(cmd[i]);
        checksum += cmd[i];
    }
    
    SPI_WriteByte(~checksum + 1); // Data Checksum (DCS)
    SPI_WriteByte(0x00);          // Postamble
    
    PN532_CS_PIN = 1; // Deselect PN532
}

/**
 * Read the response frame from PN532 after a command is sent.
 * @param buf: Buffer to store the received data.
 * @param len: Number of bytes to read.
 */
void PN532_ReadResponse(unsigned char *buf, unsigned char len) {
    PN532_CS_PIN = 0; // Select PN532
    __delay_us(10);
    SPI_WriteByte(PN532_SPI_DATAREAD); // SPI Header: Data Read (0x03)
    
    for(int i=0; i<len; i++) {
        buf[i] = SPI_ReadByte();
    }
    PN532_CS_PIN = 1; // Deselect PN532
}

/**
 * Initialize PN532 by sending SAMConfiguration command.
 * Configures the Secure Access Module (SAM) to Normal Mode.
 * @return: 1 if initialization successful, 0 otherwise.
 */
unsigned char PN532_Init(void) {
    // Command: SAMConfiguration, Mode=Normal (0x01), Timeout=50ms (0x14), IRQ=On (0x01)
    unsigned char cmd[] = {PN532_COMMAND_SAMCONFIGURATION, 0x01, 0x14, 0x01};
    unsigned char dummy[20];
    
    PN532_SendCommand(cmd, sizeof(cmd));
    
    if(!PN532_WaitReady()) {
        return 0; // PN532 failed to acknowledge or process the command
    }
    
    // Read the ACK and response to clear the buffer
    PN532_ReadResponse(dummy, 15); 
    return 1;
}

/**
 * Read the Unique Identifier (UID) of a passive target (NFC/RFID tag).
 * Uses InListPassiveTarget command to scan for 106kbps Type A cards.
 * @param uid: Buffer to store the detected UID.
 * @param uidLen: Pointer to store the length of the UID.
 * @return: 1 if a card is successfully detected and UID read, 0 otherwise.
 */
unsigned char PN532_ReadUID(unsigned char *uid, unsigned char *uidLen) {
    // Command: InListPassiveTarget, MaxTargets=1, BaudRate=106kbps TypeA (0x00)
    unsigned char cmd[] = {PN532_COMMAND_INLISTPASSIVETARGET, 0x01, 0x00};
    unsigned char ackBuffer[10];
    unsigned char buf[32];
    
    // 1. Send the command to PN532
    PN532_SendCommand(cmd, sizeof(cmd));
    
    // 2. Wait for PN532 to receive the command
    if(!PN532_WaitReady()) return 0; 
    
    // 3. Read ACK (6 bytes: 00 00 FF 00 FF 00) to confirm command reception
    PN532_ReadResponse(ackBuffer, 6); 
    
    // 4. Wait for the actual card scanning result
    if(!PN532_WaitReady()) return 0; 
    
    // 5. Read the data frame containing card info
    PN532_ReadResponse(buf, 26);
    
    // 6. Search for the Start of Frame (0xD5) in the buffer
    int offset = 0;
    while(offset < 20 && buf[offset] != 0xD5) offset++;
    
    if(offset >= 20) return 0; // Start of Frame header not found
    
    // Check if at least one card was found (buf[offset+2] is NbTg)
    if(buf[offset+2] != 1) return 0; 
    
    *uidLen = buf[offset+6]; // Extract NFCID Length (typically 4 or 7 bytes)
    
    // 7. Copy the UID bytes to the output buffer
    for(int i=0; i<*uidLen; i++) {
        uid[i] = buf[offset+7+i];
    }
    
    return 1;
}