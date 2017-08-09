
#ifndef __BOUCLE_H__
#define __BOUCLE_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>


#define PERIPHERALS_SERVICE_TIMEOUT    100



void boucle_simu ();

void service_peripherals (uint8_t force = 0);

#endif
