#include "light.h"

void light_init(void){
    light_TRIS = 0;
    light_LAT = 0;
}                         
void light_start(void){
    light_LAT = 1;
}                        
void light_stop(void){
    light_LAT = 0;
} 
