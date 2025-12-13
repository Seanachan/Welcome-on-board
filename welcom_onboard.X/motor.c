#include <ctype.h>
#include <stdlib.h>
#include <stdlib.h>
#include "stdio.h"
#include "string.h"
#include <xc.h>
#include "seg7/seg7.h"
#include "motor.h"
int speed = 0;
void CCP_Seg7_Initialize(void)
{
    // general
    ADCON1 = 0x0F; // all digital so seg7 on RA0/RA1 works

    // LED
    TRISD = 0; // 0:output 1:input
    LATD = 0;  // value = 0 -> dark

    // button (fixed high priority)
    INTCONbits.INT0IF = 0; // flag bit
    TRISBbits.RB0 = 1;     // RB0(INT0) : input
    INTCONbits.INT0IE = 1; // Enable RB0

    INTERRUPT_Initialize();
    CCP_Initialize();
    seg7_init();
    seg7_setBrightness(7);
}
void CCP_Initialize()
{
    TRISCbits.TRISC2 = 0; // CCP1 (RC2) PWM output
    TRISCbits.TRISC1 = 0; // CCP2 (RC1) PWM output

    CCP1CONbits.CCP1M = 0b1100;
    CCP2CONbits.CCP2M = 0b1100;

    T2CONbits.T2CKPS = 0b01;
    PR2 = 49;
    T2CONbits.TMR2ON = 0b1;

    // Only touch the PWM pin; leave RC6/RC7 for UART
    LATCbits.LATC2 = 0;
}

void INTERRUPT_Initialize(void)
{
    // Global
    //(1)oper priority
    RCONbits.IPEN = 1;   // enable Interrupt Priority mode
    INTCONbits.GIEH = 1; // enable high priority interrupt
    INTCONbits.GIEL = 1; // enable low priority interrupt
}

void ADC_Initialize(void)
{
    // Set RA0(AN0) as analog input(variable resistor)
    TRISAbits.RA0 = 1;        // Set RA0 as input port
    ADCON1bits.PCFG = 0b1110; // AN0 as analog input, others as digital
    ADCON0bits.CHS = 0b0000;  // Select AN0 channel

    // step1 (only change ADCS ACQT)
    ADCON1bits.VCFG0 = 0;    // Vref+ = Vdd
    ADCON1bits.VCFG1 = 0;    // Vref- = Vss
    ADCON2bits.ADCS = 0b100; // Tad = 4*Tosc
    ADCON2bits.ACQT = 0b010; // Tad = 1 us acquisition time set 4Tad = 4 > 2.4
    ADCON0bits.ADON = 1;     // Enable ADC
    ADCON2bits.ADFM = 1;     // right justified

    // step2
    PIE1bits.ADIE = 0;   // Enable ADC interrupt
    PIR1bits.ADIF = 0;   // Clear ADC interrupt flag
    IPR1bits.ADIP = 0;   // low priority
    INTCONbits.PEIE = 1; // Enable peripheral interrupts
    INTCONbits.GIE = 1;  // Enable global interrupts

    // step3
    ADCON0bits.GO = 1; // Stop ADC conversion
}

void forward()
{
    LATDbits.LATD7 = 1;
    LATDbits.LATD6 = 0;
    LATDbits.LATD5 = 1;
    LATDbits.LATD4 = 0;
}

void GOGO()
{
    speed = 40;
    forward();
    seg7_display4(gear[3][0], gear[3][1], gear[3][2], gear[3][3]);
}

void backward()
{
    LATDbits.LATD7 = 0;
    LATDbits.LATD6 = 1;
    LATDbits.LATD5 = 0;
    LATDbits.LATD4 = 1;
    seg7_display4(gear[1][0], gear[1][1], gear[1][2], gear[1][3]);
}

void highSpeed()
{
    speed = 60;
    seg7_display4(gear[2][0], gear[2][1], gear[2][2], gear[2][3]);
}

void lowSpeed()
{
    speed = 40;
    seg7_display4(gear[3][0], gear[3][1], gear[3][2], gear[3][3]);
}

void park()
{
    speed = 0;
    seg7_display4(gear[0][0], gear[0][1], gear[0][2], gear[0][3]);
}

void turnLeft()
{ // speed = 40 , times = 1000 ;
    LATDbits.LATD7 = 1;
    LATDbits.LATD6 = 0;
    LATDbits.LATD5 = 0;
    LATDbits.LATD4 = 0;
    for (int i = 0; i < 1000; i++)
    {
        __delay_ms(1);
        CCPR1L = 40;
        CCP1CONbits.DC1B = 0;
        CCPR2L = 40;
        CCP2CONbits.DC2B = 0;
    }
    forward();
}

void turnRight()
{
    LATDbits.LATD7 = 0;
    LATDbits.LATD6 = 0;
    LATDbits.LATD5 = 1;
    LATDbits.LATD4 = 0;
    for (int i = 0; i < 1000; i++)
    {
        __delay_ms(1);
        CCPR1L = 40;
        CCP1CONbits.DC1B = 0;
        CCPR2L = 40;
        CCP2CONbits.DC2B = 0;
    }
    forward();
}
