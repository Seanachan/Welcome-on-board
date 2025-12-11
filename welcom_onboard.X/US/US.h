#ifndef US_H
#define US_H

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 4000000

#define US_TRIG_LAT LATAbits.LATA2
#define US_TRIG_TRIS TRISAbits.TRISA2
#define US_ECHO_PORT PORTBbits.RB0
#define US_ECHO_TRIS TRISBbits.TRISB0

void US_Init(void);
void US_Trigger(void);           // emit US pulse
uint16_t US_GetDistance(void);  // get distance(cm)

extern volatile uint16_t distance;  

#endif
