#include <ctype.h>
#include <stdlib.h>
#include <stdlib.h>
#include "stdio.h"
#include "string.h"
#include <xc.h>
#include "seg7/seg7.h"

void CCP_Initialize()
{
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
    // default 1Mhz
    //(1)
    IRCF2 = 1;
    IRCF1 = 1;
    IRCF0 = 0;
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
    PIE1bits.ADIE = 1;   // Enable ADC interrupt
    PIR1bits.ADIF = 0;   // Clear ADC interrupt flag
    IPR1bits.ADIP = 1;   // high priority
    INTCONbits.PEIE = 1; // Enable peripheral interrupts
    INTCONbits.GIE = 1;  // Enable global interrupts

    // step3
    ADCON0bits.GO = 1; // Stop ADC conversion
}

void SYSTEM_Initialize(void)
{
    // general
    ADCON1 = 0x0E; // set as digital -> reset flag bit DO FIRST!!

    // LED
    TRISD = 0; // 0:output 1:input
    LATD = 0;  // value = 0 -> dark

    // button (fixed high priority)
    INTCONbits.INT0IF = 0; // flag bit
    TRISBbits.RB0 = 1;     // RB0(INT0) : input
    INTCONbits.INT0IE = 1; // Enable RB0

    OSCILLATOR_Initialize();
    INTERRUPT_Initialize();
    CCP_Initialize();
    seg7_init();
    seg7_setBrightness(7);
    ADC_Initialize();
}

int speed = 0;

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

const int array[] = {0, 40, 50, 60};

int state = -1;

long long adc_sum = 0;
int sum_cnt = 0;

void __interrupt(high_priority) Hi_ISR(void)
{

    if (PIR1bits.ADIF)
    {
        long long value = (ADRESH << 8) | ADRESL;
        // do sth
        adc_sum += value;
        sum_cnt++;
        if (sum_cnt >= 20)
        {
            long long average_value = adc_sum / 20;
            seg7_displayNumber(average_value);
            __delay_ms(500);
            adc_sum = 0;
            sum_cnt = 0;
        }

        PIR1bits.ADIF = 0; // clear flag bit

        // step5 & go back step3
        __delay_ms(3); // delay at least 2tad (4M ver.)
        ADCON0bits.GO = 1;
    }

    return;
}

void main(void)
{
    SYSTEM_Initialize();
    seg7_display4(gear[0][0], gear[0][1], gear[0][2], gear[0][3]);
    while (1)
    {
        CCPR1L = speed;
        CCP1CONbits.DC1B = 0;
        CCPR2L = speed;
        CCP2CONbits.DC2B = 0;
    }

    while (1)
        ;
    return;
}