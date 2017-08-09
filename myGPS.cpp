/*
 * myGPS.cpp
 *
 *  Created on: 9 août 2017
 *      Author: Vincent
 */

#include "myGPS.h"
#include "WProgram.h"

/**
 *
 */
myGPS::myGPS(HardwareSerial *ser) : TinyGPSPlus(), pmkt(ser) {

}

/**
 *
 */
void myGPS::sendCommand(const char *str) {

	this->pmkt.sendCommand(str);

}

