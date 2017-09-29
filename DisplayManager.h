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


/**
 * MODE_MENU always has to equal I_MODE_MENU
 */
enum MODE_ECRAN {
  MODE_MENU    = I_MODE_MENU,
  MODE_SD      = 1,
  MODE_GPS     = 2,
  MODE_CRS     = 3,
  MODE_PAR     = 4,
  MODE_HRM     = 5,
  MODE_HT      = 6,
  MODE_SIMU    = 7,
};

/**
 *
 */
class DisplayManager: public TLCD {
public:
	DisplayManager();

	void runCalcul();
	void runAffi(bool force = 0);

	void buttonEvent (uint8_t evt);

	bool isMenuSelected();
	void machineEtat ();

	uint8_t getPendingAction() {return _pendingAction;}

	static DisplayManager* pDisplayManager;
protected:
	uint8_t _needs_refresh;


private:
	void buttonDownEvent ();
	void buttonUpEvent ();
	void buttonPressEvent ();

	uint8_t _pendingAction;
};


#endif /* DISPLAYMANAGER_H_ */
