#define _XTAL_FREQ 4000000

// --- Config for Software UART Pin ---
#define SW_TX_PIN LATBbits.LATB4 // Using RB4 as TX
#define SW_TX_TRIS TRISBbits.TRISB4

// --- Software UART Functions ---

void SoftUART_Init(void)
{
  SW_TX_TRIS = 0; // Set RB4 as Output
  SW_TX_PIN = 1;  // Idle state for UART is HIGH
}

void SoftUART_Write(unsigned char data)
{
  // 1. Disable Interrupts to protect timing
  // (If you are using interrupts for Bluetooth, this pauses them briefly)
  unsigned char interrupt_status = INTCONbits.GIE;
  INTCONbits.GIE = 0;

  // 2. Start Bit (Line goes LOW)
  SW_TX_PIN = 0;
  __delay_us(104); // 104us for 9600 baud

  // 3. Data Bits (8 bits, LSB first)
  for (int i = 0; i < 8; i++)
  {
    if (data & 1)
    {
      SW_TX_PIN = 1;
    }
    else
    {
      SW_TX_PIN = 0;
    }
    data >>= 1;      // Shift to next bit
    __delay_us(104); // Wait 1 bit time
  }

  // 4. Stop Bit (Line goes HIGH)
  SW_TX_PIN = 1;
  __delay_us(104);

  // 5. Restore Interrupts
  INTCONbits.GIE = interrupt_status;
}

// --- DFPlayer Logic (Uses SoftUART) ---

void Send_DFPlayer_Command(unsigned char command, unsigned int parameter)
{
  // Standard DFPlayer Command Structure
  unsigned char cmd_data[6] = {
      0xFF, 0x06, command, 0x00,
      (unsigned char)(parameter >> 8),
      (unsigned char)(parameter & 0xFF)};

  // Calculate Checksum
  unsigned int sum = 0;
  for (int i = 0; i < 6; i++)
  {
    sum += cmd_data[i];
  }
  unsigned int checksum = 0xFFFF - sum + 1;

  // Send the 10-byte packet using Software UART
  SoftUART_Write(0x7E); // Start
  for (int i = 0; i < 6; i++)
  {
    SoftUART_Write(cmd_data[i]);
  }
  SoftUART_Write((unsigned char)(checksum >> 8));   // Checksum H
  SoftUART_Write((unsigned char)(checksum & 0xFF)); // Checksum L
  SoftUART_Write(0xEF);                             // End

  // Allow time for DFPlayer to process
  __delay_ms(50);
}

// --- High-Level wrappers for your Logic ---

void mp3_Init()
{
  SoftUART_Init();
  __delay_ms(1000);                // Wait for DFPlayer power up
  Send_DFPlayer_Command(0x06, 25); // Set Volume to 25
}

void mp3_Play(int track)
{
  Send_DFPlayer_Command(0x03, track);
}

void mp3_Stop()
{
  Send_DFPlayer_Command(0x16, 0);
}