/*
 * myGPS.h
 *
 *  Created on: 9 août 2017
 *      Author: Vincent
 */

#ifndef MYGPS_H_
#define MYGPS_H_

#include <Adafruit_GPS.h>
#include <TinyGPS++.h>

class myGPS : public TinyGPSPlus {
public:
	myGPS(HardwareSerial *ser);
	void sendCommand(const char *);



private:

	Adafruit_GPS pmkt;

};


#endif /* MYGPS_H_ */
