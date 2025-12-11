#pragma config OSC = INTIO67      // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = ON       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 1        // Watchdog Timer Postscale Select bits (1:1)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF         // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)


#include <ctype.h>
#include <stdlib.h>
#include <stdlib.h>
#include "stdio.h"
#include "string.h"
#include <xc.h>
#include "seg7.h"

#define _XTAL_FREQ 4000000

//IN1 = RD4
//IN2 = RD5
//IN3 = RD6
//IN4 = RD7

void CCP_Initialize() {
    TRISCbits.TRISC2 = 0; 
    
    CCP1CONbits.CCP1M = 0b1100;  
    CCP2CONbits.CCP2M = 0b1100;
    
    T2CONbits.T2CKPS = 0b01; 
    PR2 = 49;  
    T2CONbits.TMR2ON = 0b1; 
    
    TRISC = 0;
    LATC = 0;
}

void OSCILLATOR_Initialize(void)
{
    //default 1Mhz
    //(1)
    IRCF2 = 1; 
    IRCF1 = 1;
    IRCF0 = 0;
}
void INTERRUPT_Initialize (void)
{
    //Global
    //(1)oper priority
    RCONbits.IPEN = 1;      //enable Interrupt Priority mode
    INTCONbits.GIEH = 1;    //enable high priority interrupt
    INTCONbits.GIEL = 1;     //enable low priority interrupt
    
    
}

void ADC_Initialize (void)
{
    // Set RA0(AN0) as analog input(variable resistor)
    TRISAbits.RA0 = 1;         // Set RA0 as input port
    ADCON1bits.PCFG = 0b1110;  // AN0 as analog input, others as digital
    ADCON0bits.CHS = 0b0000;   // Select AN0 channel

    // step1 (only change ADCS ACQT)
    ADCON1bits.VCFG0 = 0;     // Vref+ = Vdd
    ADCON1bits.VCFG1 = 0;     // Vref- = Vss
    ADCON2bits.ADCS = 0b100;  // Tad = 4*Tosc 
    ADCON2bits.ACQT = 0b010;  // Tad = 1 us acquisition time set 4Tad = 4 > 2.4
    ADCON0bits.ADON = 1;      // Enable ADC
    ADCON2bits.ADFM = 1;      // right justified

    // step2
    PIE1bits.ADIE = 1;    // Enable ADC interrupt
    PIR1bits.ADIF = 0;    // Clear ADC interrupt flag
    IPR1bits.ADIP = 1;    // high priority
    INTCONbits.PEIE = 1;  // Enable peripheral interrupts
    INTCONbits.GIE = 1;   // Enable global interrupts

    // step3
    ADCON0bits.GO = 1;  // Stop ADC conversion
}

void SYSTEM_Initialize(void)
{
    //general
    ADCON1 = 0x0E;  //set as digital -> reset flag bit DO FIRST!!
    
    //LED
    TRISD = 0;  // 0:output 1:input
    LATD = 0;   // value = 0 -> dark
    
    //button (fixed high priority)
    INTCONbits.INT0IF = 0;  //flag bit
    TRISBbits.RB0 = 1;   // RB0(INT0) : input
    INTCONbits.INT0IE = 1;  //Enable RB0
    
    OSCILLATOR_Initialize();
    INTERRUPT_Initialize();
    CCP_Initialize();
    seg7_init();
    seg7_setBrightness(7);
    ADC_Initialize();

}

int speed = 0;

void forward(){
    LATDbits.LATD7 = 1;
    LATDbits.LATD6 = 0;
    LATDbits.LATD5 = 1;
    LATDbits.LATD4 = 0;
}

void GOGO(){
    speed = 40;
    forward();
    seg7_display4(gear[3][0], gear[3][1], gear[3][2], gear[3][3]);
}

void backward(){
    LATDbits.LATD7 = 0;
    LATDbits.LATD6 = 1;
    LATDbits.LATD5 = 0;
    LATDbits.LATD4 = 1;
    seg7_display4(gear[1][0], gear[1][1], gear[1][2], gear[1][3]);
}

void highSpeed(){
    speed = 60;
    seg7_display4(gear[2][0], gear[2][1], gear[2][2], gear[2][3]);
}

void lowSpeed(){
    speed = 40;
    seg7_display4(gear[3][0], gear[3][1], gear[3][2], gear[3][3]);
}

void park(){
    speed = 0;
    seg7_display4(gear[0][0], gear[0][1], gear[0][2], gear[0][3]);
}

void turnLeft(){// speed = 40 , times = 1000 ; 
    LATDbits.LATD7 = 1;
    LATDbits.LATD6 = 0;
    LATDbits.LATD5 = 0;
    LATDbits.LATD4 = 0;
    for(int i = 0; i < 1000; i++){
        __delay_ms(1);
        CCPR1L = 40;
        CCP1CONbits.DC1B = 0;
        CCPR2L = 40;
        CCP2CONbits.DC2B = 0;
    }
    forward();
}

void turnRight(){
    LATDbits.LATD7 = 0;
    LATDbits.LATD6 = 0;
    LATDbits.LATD5 = 1;
    LATDbits.LATD4 = 0;
    for(int i = 0; i < 1000; i++){
        __delay_ms(1);
        CCPR1L = 40;
        CCP1CONbits.DC1B = 0;
        CCPR2L = 40;
        CCP2CONbits.DC2B = 0;
    }
    forward();
}

const int array[] = {0, 40, 50, 60};


int state = -1;

long long adc_sum = 0;
int sum_cnt = 0;

void __interrupt(high_priority) Hi_ISR(void)
{
    //------ Button ------
//    if(INTCONbits.INT0IF == 1){
//        //Handle Bouncing (release-trigger)
//        __delay_ms(25);
//        while(PORTBbits.RB0 == 0);
//        __delay_ms(25);
//        
//        state = (state + 1) % 7;
//        LATC = (state << 4);
//        if(state == 0) {
//            GOGO();
//        }
//        if(state == 1) {
//            lowSpeed();
//        }
//        if(state == 2) {
//            highSpeed();
//        }
//        if(state == 3) {
//            turnLeft();
//        }
//        if(state == 4) {
//            turnRight();
//        }
//        if(state == 5) {
//            backward();
//        }
//        if(state == 6) {
//            park();
//        }
//        speed = array[state];
        
//        INTCONbits.INT0IF = 0;
//        return;
//    }
    
    if (PIR1bits.ADIF) {
        long long value = (ADRESH << 8) | ADRESL;
        //do sth
        adc_sum += value;
        sum_cnt++;
        if(sum_cnt >= 20){
            long long average_value = adc_sum / 20;
            seg7_displayNumber(average_value);
            __delay_ms(500);
            adc_sum = 0;
            sum_cnt = 0;
        }
        
        
        PIR1bits.ADIF = 0; // clear flag bit

        // step5 & go back step3
        __delay_ms(3);  // delay at least 2tad (4M ver.)
        ADCON0bits.GO = 1;
    }
    
    return;
}



void main(void) 
{
    SYSTEM_Initialize();
    seg7_display4(gear[0][0], gear[0][1], gear[0][2], gear[0][3]);
    while(1){
        CCPR1L = speed;
        CCP1CONbits.DC1B = 0;
        CCPR2L = speed;
        CCP2CONbits.DC2B = 0;
    }
    
    while(1);
    return;
}