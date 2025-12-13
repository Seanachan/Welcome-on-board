#ifndef light_H
#define light_H

#include <xc.h>
#include <stdint.h>

// ===== ???? =====
#define light_LAT LATBbits.LATB1
#define light_TRIS TRISBbits.TRISB1


// ===== function declaration =====
void light_init(void);                         // init pin
void light_start(void);                        // start signal
void light_stop(void); 

#endif