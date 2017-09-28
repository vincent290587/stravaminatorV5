/*
 * DisplayManager.cpp
 *
 *  Created on: 28 sept. 2017
 *      Author: Vincent
 */

#include "DisplayManager.h"
#include "Boucle.h"
#include "BoucleCRS.h"
#include "Global.h"

using namespace mvc;


DisplayManager* DisplayManager::pDisplayManager = nullptr;

static void callbackMENU(int entier) {
	// empty callback
	uint8_t mode_cur = DisplayManager::pDisplayManager->getModeAffi();

	if (mode_cur == MODE_MENU) {
		// go back to last mode
		uint8_t mode_prec = DisplayManager::pDisplayManager->getModeAffiPrec();
		DisplayManager::pDisplayManager->setModeAffi(mode_prec == MODE_MENU ? MODE_CRS : mode_prec);
	} else {
		// save current mode
		DisplayManager::pDisplayManager->setModeAffiPrec(mode_cur);
		// reset selection
		DisplayManager::pDisplayManager->resetSelectionMenu();
		// dispay menu
		DisplayManager::pDisplayManager->setModeAffi(MODE_MENU);
	}
}

static void callbackCRS(int entier) {
	// empty callback
	DisplayManager::pDisplayManager->setModeAffi(MODE_CRS);
}

static void callbackPRC(int entier) {
	// empty callback
	DisplayManager::pDisplayManager->setModeAffi(MODE_PAR);
}

static void callbackHRM(int entier) {
	// empty callback
	DisplayManager::pDisplayManager->setModeAffi(MODE_HRM);
}

static void callbackHT(int entier) {
	// empty callback
	DisplayManager::pDisplayManager->setModeAffi(MODE_HT);
}

static void callbackSimu(int entier) {
	// empty callback
	DisplayManager::pDisplayManager->setModeAffi(MODE_SIMU);
}

DisplayManager::DisplayManager(): TLCD(SHARP_CS) {

	pDisplayManager = this;

	this->setModeAffi(MODE_SD);
	this->setModeCalcul(MODE_GPS);

	// init all the menu items
	sIntelliMenuItem item;

	item.name = "Retour";
	item.p_func = callbackMENU;
	this->addMenuItem(&item);

	item.name = "Mode CRS";
	item.p_func = callbackCRS;
	this->addMenuItem(&item);

	item.name = "Mode PRC";
	item.p_func = callbackPRC;
	this->addMenuItem(&item);

	item.name = "Mode HRM";
	item.p_func = callbackHRM;
	this->addMenuItem(&item);

	item.name = "Mode HT";
	item.p_func = callbackHT;
	this->addMenuItem(&item);

	item.name = "Mode simu";
	item.p_func = callbackSimu;
	this->addMenuItem(&item);

}


void DisplayManager::runCalcul() {

	// aiguillage pour chaque type d'affichage
	switch (display.getModeCalcul()) {
	case MODE_GPS:
		display.setNbSatIV(String(satsInView.value()).toInt());
		display.setNbSatU((uint16_t)gps.satellites.value());
		display.setHDOP(String(hdop.value()).toInt());

		// no break;

	case MODE_CRS:

		if (mode_simu) {
			if (is_gps_awake) {
				gps.sendCommand(PMTK_STANDBY);
				is_gps_awake = 0;
			}
		} else if (!is_gps_awake) {
			gps.sendCommand(PMTK_AWAKE);
			is_gps_awake = 1;
		}

		if (updateLocData()) {
			// no useable LOC
			return;
		}

		// maj baro
		if (mode_simu || att.nbpts <= MIN_POINTS) {
			att.alt = att.gpsalt;
		} else {
			att.alt = baro.pressureToAltitude(fpressu.output());
		}

		mes_points.enregistrePos(att.lat, att.lon, att.alt, att.secj);

		// maj merites
		if (att.nbpts < MIN_POINTS) {

			return;

		} else if (att.nbpts == MIN_POINTS) {
			// maj GPS
			gps.sendCommand("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28");
			// maj BMP
			updateAltitudeOffset();
			// mode switch
			display.setModeCalcul(MODE_CRS);
			display.setModeAffi(MODE_CRS);
			att.secj_prec = att.secj;
			att.nbsec_act = 1;
		}

		att.dist = cumuls.getDistance();
		att.climb = cumuls.getClimb();

		boucle_outdoor();

		if (cumuls.majMerite(att.lat, att.lon, att.alt) == 1) {

			loggerData();
			att.nbsec_act += att.secj - att.secj_prec;
			att.secj_prec = att.secj;

		} else if ((!is_cad_init && att.speed < 3.) ||
				(is_cad_init && att.cad_speed < 3.)) {

			// a l'arret
			att.secj_prec = att.secj;

		}

		att.vit_asc = cumuls.getVitAsc();
		att.pwr = cumuls.getPower();
		att.climb = cumuls.getClimb();

		break;

	case MODE_SIMU:
		boucle_simu();
		// no break
	case MODE_HRM:

		if (is_gps_awake) {
			gps.sendCommand(PMTK_STANDBY);
			is_gps_awake = 0;
		}

		att.secj = millis() / 1000;

		if (cumuls.majCRS(att.cad_speed, att.bpm, 0)) {

			loggerRR();

			att.nbsec_act += att.secj - att.secj_prec;
			att.secj_prec = att.secj;

		}

		break;
	case MODE_HT:

		if (is_gps_awake) {
			gps.sendCommand(PMTK_STANDBY);
			is_gps_awake = 0;
		}

		att.secj = millis() / 1000;

		if (cumuls.majCRS(att.cad_speed, att.bpm, 0)) {

			loggerHT();

			att.nbsec_act += att.secj - att.secj_prec;
			att.secj_prec = att.secj;

			att.dist = cumuls.getDistance();

		}

		if (att.cad_speed < 3.) {

			att.secj_prec = att.secj;

		}

		break;
	}

}


void DisplayManager::runAffi(bool force) {

	this->machineEtat();

	if (_needs_refresh || force) {

		_needs_refresh = 0;

		this->resetBuffer();

		// si pas de fix on affiche la page d'info GPS
		if (att.nbpts > MIN_POINTS + 10 && this->getModeCalcul() != MODE_HRM && this->getModeCalcul() != MODE_HT) {
			// loc is not good anymore
			if (isLocOutdated() && this->getModeCalcul() == MODE_CRS) {
				gps.sendCommand("$PMTK314,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29");
				display.setStoredMode(this->getModeCalcul());
				display.setModeCalcul(MODE_GPS);
				display.setModeAffi(MODE_GPS);
			} else if (!isLocOutdated() && this->getModeCalcul() == MODE_GPS) {
				// loc is good again
				gps.sendCommand("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28");
				this->setModeCalcul(MODE_CRS);
				this->setModeAffi(MODE_CRS);
			}
		}

		switch (this->getModeAffi()) {
		case MODE_SD:
			this->afficheBoot();
			break;
		case MODE_GPS:
			this->afficheGPS();
			break;
		case MODE_CRS:
			this->setModeCalcul(MODE_CRS);
			this->afficheSegments();
			break;
		case MODE_PAR:
			this->setModeCalcul(MODE_CRS);
			if (_par_act == 0) {
				this->setModeAffi(MODE_CRS);
				this->afficheSegments();
			} else {
				this->afficheParcours();
			}
			break;
		case MODE_HRM:
			this->setModeCalcul(MODE_HRM);
			this->afficheHRM();
			break;
		case MODE_HT:
			this->setModeCalcul(MODE_HT);
			this->afficheHT();
			break;
		case MODE_SIMU:
			this->setModeCalcul(MODE_SIMU);
			this->afficheHT();
			break;
		case MODE_MENU:
			this->affichageMenu ();
			break;
		}


		this->affiANCS();
		this->writeWhole();

	}
}

void DisplayManager::buttonEvent (uint8_t evt) {
	switch (evt) {
	case 3:
		this->buttonPressEvent();
		break;
	case 2:
		this->buttonDownEvent();
		break;
	case 1:
		this->buttonUpEvent ();
		break;
	}
}

void DisplayManager::buttonDownEvent () {

	if (!_pendingAction) {
		_pendingAction = BUTTON_DOWN;
	}

}

void DisplayManager::buttonUpEvent () {

	if (!_pendingAction) {
		_pendingAction = BUTTON_UP;
	}

}


void DisplayManager::buttonPressEvent () {

	if (!_pendingAction) {
		_pendingAction = BUTTON_PRESS;
	}

}

bool DisplayManager::isMenuSelected() {
	return this->getModeAffi() == MODE_MENU;
}
