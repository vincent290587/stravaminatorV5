/*
 * BoucleCRS.h
 *
 *  Created on: 9 août 2017
 *      Author: Vincent
 */

#ifndef BOUCLECRS_H_
#define BOUCLECRS_H_


#define SEG_DISPLAY_DIST  150.


void boucle_outdoor ();

uint8_t updateLocData();

uint8_t isLocOutdated();

void updateAltitudeOffset ();

#endif /* BOUCLECRS_H_ */
