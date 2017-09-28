/*
 * DisplayManager.h
 *
 *  Created on: 28 sept. 2017
 *      Author: Vincent
 */

#ifndef DISPLAYMANAGER_H_
#define DISPLAYMANAGER_H_


#include "WProgram.h"
#include "TLCD.h"


enum MODE_ECRAN {
  MODE_MENU    = 0,
  MODE_SD      = 1,
  MODE_GPS     = 2,
  MODE_CRS     = 3,
  MODE_PAR     = 4,
  MODE_HRM     = 5,
  MODE_HT      = 6,
  MODE_SIMU    = 7,
};

class DisplayManager: public TLCD {
public:
	DisplayManager();

	void runCalcul();
	void runAffi(bool force = 0);

	void buttonEvent (uint8_t evt);

	bool isMenuSelected();

	static DisplayManager* pDisplayManager;
protected:



private:
	void buttonDownEvent ();
	void buttonUpEvent ();
	void buttonPressEvent ();

};


#endif /* DISPLAYMANAGER_H_ */
