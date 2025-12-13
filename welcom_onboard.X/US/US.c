#include <xc.h>
#include <stdint.h>
#include "US.h"

#define _XTAL_FREQ 4000000  // 4 MHz crystal

// --- Global variables ---
volatile uint16_t distance = 0;         // measured distance in cm
volatile uint32_t tmr_ticks = 0;        // accumulated Timer1 ticks
volatile uint16_t start_time = 0;       // Timer1 value at rising edge
volatile uint16_t end_time = 0;         // Timer1 value at falling edge
volatile uint8_t measuring = 0;         // flag: measuring in progress

// --- Initialize ultrasonic sensor ---
void US_Init(void) {
    US_TRIG_TRIS = 0;  // TRIG as output
    US_ECHO_TRIS = 1;  // ECHO as input
    US_TRIG_LAT = 0;

    // Timer1 configuration: Fosc/4, prescaler 1:1, 16-bit timer
    T1CON = 0x01;       // Timer1 ON, prescaler 1:1
    TMR1H = 0;
    TMR1L = 0;
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;   // enable Timer1 overflow interrupt
    IPR1bits.TMR1IP = 1;   // high priority

    // External interrupt INT0 for ECHO
    INTCONbits.INT0IF = 0;    // clear INT0 flag
    INTCONbits.INT0IE = 1;    // enable INT0 interrupt
    INTCON2bits.INTEDG0 = 1;  // rising edge trigger

    // Enable global interrupts
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

// --- Trigger ultrasonic pulse ---
void US_Trigger(void) {
    US_TRIG_LAT = 0;
    __delay_us(2);    // small delay for stability
    US_TRIG_LAT = 1;
    __delay_us(10);   // 10us pulse
    US_TRIG_LAT = 0;

    measuring = 1;
    distance = 0;

    // reset Timer1 accumulator
    TMR1H = 0;
    TMR1L = 0;
    tmr_ticks = 0;
}

// --- Interrupt Service Routine ---
//void __interrupt(high_priority) ISR(void) {
//    // INT0: ECHO signal
//    if(INTCONbits.INT0IF) {
//        if(measuring) {
//            if(INTCON2bits.INTEDG0) {  // Rising edge
//                start_time = TMR1;
//                tmr_ticks = 0;          // reset overflow count
//                INTCON2bits.INTEDG0 = 0; // next: falling edge
//            } else {                     // Falling edge
//                end_time = TMR1;
//                // compute total ticks including overflow
//                long long total_ticks;
//                if(end_time >= start_time) {
//                    total_ticks = (tmr_ticks + end_time) - start_time;
//                } else {
//                    // Timer1 rolled over between rising and falling edge
//                    total_ticks = (tmr_ticks + 65536 + end_time) - start_time;
//                }
//                distance = total_ticks / 14;   // convert to cm
//                if(distance > 400) distance = 400; // limit max distance
//                if(distance < 2) distance = 2;     // limit min distance
//                measuring = 0;
//                INTCON2bits.INTEDG0 = 1;          // next: rising edge
//            }
//        }
//        INTCONbits.INT0IF = 0; // clear INT0 flag
//    }
//
//    // Timer1 overflow
//    if(PIR1bits.TMR1IF) {
//        tmr_ticks += 65536;      // accumulate overflow ticks
//        PIR1bits.TMR1IF = 0;     // clear Timer1 interrupt flag
//    }
//}

// --- Return measured distance ---
uint16_t US_GetDistance(void) {
    uint16_t distance_real = distance * 1.03 - 1;
    return distance_real;
}
