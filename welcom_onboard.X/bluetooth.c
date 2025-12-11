#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xc.h>

char buffer[STR_MAX];
int buffer_size = 0;

//  variables
unsigned char mode = 1;
int volume = 20; // initial volume

void set_LED(int value);

// ---------------- Settings --------------------

/*
This is the legacy code section that has been moved to appropriate files.
*/
