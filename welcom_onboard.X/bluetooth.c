// CONFIG1H
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
#pragma config PBADEN = ON    // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
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

#include <ctype.h>
// #include <pic18f4520.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xc.h>

#define _XTAL_FREQ 4000000
#define STR_MAX 100
#define VR_MAX ((1 << 10) - 1)

char buffer[STR_MAX];
int buffer_size = 0;
bool btn_interr = false;

// add mode variable
unsigned char mode = 1;
// ----- State machine for 3-LED pattern -----
void set_LED(int value);

// --- DFPlayer Functions ---
void UART_WriteByte(unsigned char b)
{
    while (!TXSTAbits.TRMT)
        ; // 等待移位暫存器空
    TXREG = b;
}

void DF_SendCommand(unsigned char cmd, unsigned int param)
{
    unsigned char buf[6] = {
        0xFF, 0x06, cmd, 0x00,
        (unsigned char)(param >> 8),
        (unsigned char)(param & 0xFF)};

    unsigned int sum = 0;
    for (int i = 0; i < 6; i++)
        sum += buf[i];
    unsigned int checksum = 0xFFFF - sum + 1;

    UART_WriteByte(0x7E); // start
    for (int i = 0; i < 6; i++)
        UART_WriteByte(buf[i]);
    UART_WriteByte((unsigned char)(checksum >> 8));
    UART_WriteByte((unsigned char)(checksum & 0xFF));
    UART_WriteByte(0xEF); // end

    __delay_ms(50);
}

void DF_Init(void)
{
    DF_SendCommand(0x06, 25); // volume = 25
}

void DF_PlayTrack1(void)
{
    DF_SendCommand(0x03, 1); // 播放 0001.mp3
}
// ---------------- Uart --------------------

void putch(char data)
{ // Output on Terminal
    if (data == '\n' || data == '\r')
    {
        while (!TXSTAbits.TRMT)
            ;
        TXREG = '\r';
        while (!TXSTAbits.TRMT)
            ;
        TXREG = '\n';
    }
    else
    {
        while (!TXSTAbits.TRMT)
            ;
        TXREG = data;
    }
}

void ClearBuffer()
{
    for (int i = 0; i < STR_MAX; i++)
        buffer[i] = '\0';
    buffer_size = 0;
}

void MyusartRead()
{
    char data = RCREG;
    if (!isprint(data) && data != '\r')
        return;
    buffer[buffer_size++] = data;
    putch(data);
}

int GetString(char *str)
{
    if (buffer[buffer_size - 1] == '\r')
    {
        buffer[--buffer_size] = '\0';
        strcpy(str, buffer);
        ClearBuffer();
        return 1;
    }
    else
    {
        str[0] = '\0';
        return 0;
    }
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

// ---------------- Settings --------------------

void Initialize(void)
{
    // Configure oscillator
    OSCCONbits.IRCF = 0b110; // 4 MHz

    // Configure I/O ports
    TRISA &= 0xC1; // Set RA1-RA5 as outputs for LED
    TRISB = 1;     // RB0 as input for button
    TRISC = 0;     // PORTC as output for servo
    LATA &= 0xC1;  // Clear RA1-RA5
    LATC = 0;      // Clear PORTC

    // Configure interrupts
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

    /* Reiceiver (input)
     RSR   : Current Data
     RCREG : Correct Data (have been processed) : read data by reading the RCREG Register
    */
    ADCON1 = 0x0F; // Set all pins to Digital Mode
    LATBbits.LATB4 = 1;
    TRISBbits.TRISB4 = 0; // Make RB4 Output
    // Start ADC conversion
    // ADCON0bits.GO = 1;
}

// ---------------- OOP --------------------

int get_LED()
{
    return (LATA >> 1);
}

void set_LED(int value)
{
    LATA = (value << 1);
}

void set_LED_separately(int a, int b, int c, int d, int e)
{
    LATA = (a << 5) + (b << 4) + (c << 3) + (d << 2) + (e << 1);
}

void set_LED_analog(int value)
{
    CCPR2L = (value >> 2);
    CCP2CONbits.DC2B = (value & 0b11);
}

void __interrupt(high_priority) H_ISR()
{
}

int delay(double sec)
{
    btn_interr = false;
    for (int i = 0; i < sec * 1000 / 10; i++)
    {
        if (btn_interr)
            return -1;
        __delay_ms(10);
    }
    return 0;
}

// --------------- TODO ------------------

void keyboard_input(char *str)
{
    printf("BT CMD: %s\n", str);

    if (strcmp(str, "FORWARD") == 0)
    {
        // move robot forward
        set_LED(0xFF); // example
    }
    else if (strcmp(str, "REVERSE") == 0)
    {
        // move robot backward
        set_LED(0x00); // example
    }
    else if (strcmp(str, "TURN_LEFT") == 0)
    {
        // steering left
    }

    else if (strcmp(str, "PLAY_MUSIC") == 0)
    {
        printf("Play music\n");
        DF_PlayTrack1(); // Play track 1
    }
    else
    {
        printf("Unknown CMD: %s\n", str);
    }
}

void main()
{
    Initialize();
    DF_Init();
    printf("PIC ready\r\n");
    char str[STR_MAX];

    while (1)
    {
        if (GetString(str))
            keyboard_input(str);
    }
}