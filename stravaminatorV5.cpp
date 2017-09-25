#include <SPI.h>
#include <Wire.h>

#include <avr/sleep.h>
#include "Boucle.h"
#include "BoucleCRS.h"
#include "Global.h"
#include "Logger.h"
#include "PCcomm.h"
#include "Intonations.h"
#include "fonctions.h"
#include "fonctionsXML.h"
#include "stravaminatorV5.h"

using namespace mvc;


void setup() {
	att.has_started = 0;
	// initialize the digital pin as an output.
	Serial.begin(115200);
	Serial3.begin(115200);
	//Serial3.attachRts(2);
	//Serial3.attachCts(14);
	pinMode(led, OUTPUT);
#ifdef __DEBUG__
	digitalWriteFast(led, HIGH);
#endif

	is_gps_awake = 1;

	is_cad_init = 0;
	last_nrf_cad = millis();

	// pseudo boutons
	pinMode(virtbtn0, OUTPUT);
	digitalWriteFast(virtbtn0, HIGH);
	pinMode(virtbtn1, OUTPUT);
	digitalWriteFast(virtbtn1, HIGH);

	memset(&att, 0, sizeof(SAttitude));

	display.begin();
	delay(5);
	display.resetBuffer();
	display.updateScreen();
	display.registerHisto(&mes_points);

#ifdef __DEBUG__
	uint8_t safe = 0;
	while (!Serial) {
		delay(1000);
		safe++;
		if (safe > 100) {
			Serial.end();
			break;
		}
	}
#endif

	Serial.println("Debut");

	// reset filters
	fpressu.reset();
	stc_cur.reset();

	// init GPS
	//gps.sendCommand("$PMTK104*37"); // full cold start
	//gps.sendCommand("$PMTK103*30"); // cold start
	delay(50);
	gps.sendCommand("$PMTK314,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29");


	/* Initialise the sensor */
	baro.begin();

	/* Initialise the sensor */
	if (!stc.begin(10))
	{
		/* There was a problem detecting the BMP085 ... check your connections */
		Serial.println("Ooops, no STC detected ... Check your wiring or I2C ADDR!");
	}

	// on demarre la SPI flash
#ifdef __SST__
	pinMode(memWp, OUTPUT);
	pinMode(memHold, OUTPUT);
	digitalWriteFast(memWp, HIGH);
	digitalWriteFast(memHold, HIGH);
	SerialFlash.begin(memCs);
	SerialFlash.globalUnlock();

	checkSST26();

#ifdef __DEBUG_SST__
	dumpLogGPS();
	SerialFlash.eraseAll();
#endif

#else
	pinMode(memWp, OUTPUT);
	pinMode(memHold, OUTPUT);
	digitalWriteFast(memWp, LOW);
	digitalWriteFast(memHold, HIGH);
#endif

	if (!sd.begin(sd_cs, SPI_HALF_SPEED)) {
		Serial.println(F("Card initialization failed."));
		display.setSD(-1);
		display.updateScreen();
		errorTone();
		display.setModeCalcul(MODE_HRM);
		display.setModeAffi(MODE_HRM);
		display.updateScreen();
	}
	else {
		Serial.println(F("Card OK."));
		display.setSD(1);
		display.updateScreen();
		delay(150);
		initListeSegments();
		Serial.print("Nombre total de segments: ");
		Serial.println(mes_segments.size());
		display.setNbSeg(mes_segments.size());
		display.updateScreen();
		delay(1000);
		display.setModeCalcul(MODE_GPS);
		display.setModeAffi(MODE_GPS);
		display.updateScreen();
		delay(400);
		Serial.println("Fin setup");
	}

	// start listening to GPS
	Serial1.begin(9600);

	service_peripherals(1);

	att.has_started = 1;

	// Initialize all the uninitialized TinyGPSCustom objects
	for (int i = 0; i < 4; ++i)
	{
		satNumber[i].begin(gps, "GPGSV", 4 + 4 * i); // offsets 4, 8, 12, 16
		snr[i].begin(gps, "GPGSV", 7 + 4 * i); // offsets 7, 11, 15, 19
	}

#ifdef __DEBUG__
	digitalWriteFast(led, LOW);
#endif
}


void serialEvent() {
	char c;
	uint8_t format = 0;
	while (Serial.available()) {
		c = Serial.read();
		if (mode_simu == 0) {
			format = nordic.encode(c);
			if (format == _SENTENCE_PC) {
				if (nordic.getPC() == 0) {
					// demande de telechargement de l'historique
					download_request = 1;
				} else if (nordic.getPC() == 1) {
					// demande de mise a jour des segments
					//upload_request = 1;
				} else if (nordic.getPC() == 2) {
					mode_simu = 1;
					Serial.println("Mode simu active");
					Serial.clear();
				} else if (nordic.getPC() == 3) {
					// fin histo
					download_request = 0;
					Serial.println("Fin du mode historique");
				} else if (nordic.getPC() == 4) {
					// fin histo
					download_request = 0;
					// effacement de l'histo
#ifndef __SST__
					effacerHisto();
#else
					setToBeErased ();
#endif
				}
			}
		} else {
			if (c == '#') {
				Serial.println("Mode simu desactive");
				mode_simu = 0;
			}

			if (nordic.encode(c)) {
				new_gpsn_data = 1;
			}
		}
	}
}

void serialEvent1() {
	char c;
	while (Serial1.available() && att.has_started > 0) {
		c = Serial1.read();
#ifdef __DEBUG_GPS_S__
		Serial.write(c);
#endif
		if (mode_simu == 0) {
			bool enc = gps.encode(c);
			if (enc && millis() - lastFix > 500) {
				if (gps.location.isValid()) {
					lastFix = millis();
					//Serial.println("Loc OK");
				}
				new_gps_data = 1;
			}
		}
	}
}

void serialEvent3() {
	char c;
	uint8_t _format = 0;
	while (Serial3.available() && att.has_started > 0) {
		c = Serial3.read();
		_format = nordic.encode(c);
#ifdef __DEBUG_NRF__
		Serial.write(c);
		if (_format != _SENTENCE_NONE) {
			Serial.println(String("\nFormat: ") + _format);
		}
#endif
		switch (_format) {
		case _SENTENCE_LOC:
			new_gpsn_data = 1;
			break;
		case _SENTENCE_HRM:
			new_hrm_data = 1;
			break;
		case _SENTENCE_CAD:
			new_cad_data = 1;
			break;
		case _SENTENCE_ANCS:
			new_ancs_data = 1;
			break;
		case _SENTENCE_BTN:
			new_btn_data = 1;
			break;
		case _SENTENCE_DBG:
			new_dbg_data = 1;
			break;
		case _SENTENCE_NONE:
			break;
		default:
			break;
		}
	}
}

/**
 *
 */
void loop() {
#ifdef __DEBUG__
	digitalWriteFast(led, LOW);
	Serial.println("LOOP");

	Serial.println(String("Satellites in view: ") + satsInView.value());
#endif

	// vidage des buffer Serials
	yield();

	// maj nordic
	if (new_btn_data) {
		new_btn_data = 0;
		buttonEvent(nordic.getBTN());
		display.updateScreen();
	}
	if (new_hrm_data) {
		att.bpm = nordic.getBPM();
		att.rrint = nordic.getRR();
	}
	if (new_cad_data && nordic.getRPM()) {
		att.cad_rpm = nordic.getRPM();
	}
	if (new_cad_data && nordic.getCadSpeed() > 0.) {
		att.cad_speed = (float)nordic.getCadSpeed() / 1000.;
		last_nrf_cad = millis();
		is_cad_init = 1;
	} else if (millis() - last_nrf_cad > CAD_SPEED_TIMEOUT_MS) {
		// if no event were recived from the CAD, set the speed to 0
		att.cad_speed = 0.;
	}
	if (new_ancs_data) {
		if (alertes_nb < 20) {
			alertes_nb += 1;
		}
		String tmp = String ("") + nordic.getANCS_title() + nordic.getANCS_msg();
		if (tmp.indexOf("ppel") > 0 || tmp.indexOf("essage") > 0) {
			basicTone();
		}
		display.notifyANCS(nordic.getANCS_type(), nordic.getANCS_title(), nordic.getANCS_msg());
	}
	if (new_dbg_data) {
		String tmp = String("Erreur nRF51: code=") + String(nordic.getDBG_code(), HEX) + " - line:" + String(nordic.getDBG_line());
		loggerMsg(tmp.c_str());
		if (nordic.getDBG_type() == 1) {
			loggerMsg(nordic.getDBG_msg());
		}
	}
	if (download_request) {
		Serial.println("Debut MaJ");
#ifndef __SST__
		if (PChisto()) {
			Serial.println("Erreur lors de la MaJ");
		}
#else
		dumpLogGPS();
#endif
	}

	// get power data
	att.cbatt = stc_cur.output();
	att.vbatt = stc.getCorrectedVoltage(BATT_INT_RES);
	att.pbatt = percentageBatt(att.vbatt);

	// get altitude data
	att.alt = baro.pressureToAltitude(fpressu.output());


#ifdef __DEBUG_STC__
	Serial.print("Temperature STC :    ");
	Serial.print(stc.getTemperature(), 2);
	Serial.println(" degres C");
	Serial.print("Current STC     :    ");
	Serial.print(att.cbatt, 2);
	Serial.println(" mA");
	Serial.print("Voltage STC     :    ");
	Serial.print(att.vbatt, 2);
	Serial.println(" V");
	Serial.print("Charge STC      :    ");
	Serial.print(stc.getCharge(), 2);
	Serial.println(" mAh");
#endif


#ifdef __DEBUG_GPS__
	static uint32_t last_snr = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (satNumber[i].isValid() && new_gps_data &&
				millis() - last_snr > 900) {
			String temp = String("Satellites signal data: sat#") + satNumber[i].value() + " SNR= " + snr[i].value();
			Serial.println(temp);
			if (i == 3) last_snr = millis();
		}
	}
#endif

	new_hrm_data = 0;
	new_cad_data = 0;
	new_ancs_data = 0;
	new_btn_data = 0;
	new_dbg_data = 0;
	download_request = 0;
	upload_request = 0;

	// si pas de fix on affiche la page d'info GPS
	if (att.nbpts > MIN_POINTS + 10 && display.getModeCalcul() != MODE_HRM && display.getModeCalcul() != MODE_HT) {
		// loc is not good anymore
		if (isLocOutdated() && display.getModeCalcul() == MODE_CRS) {
			gps.sendCommand("$PMTK314,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29");
			display.setStoredMode(display.getModeCalcul());
			display.setModeCalcul(MODE_GPS);
			display.setModeAffi(MODE_GPS);
		} else if (!isLocOutdated() && display.getModeCalcul() == MODE_GPS) {
			// loc is good again
			gps.sendCommand("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28");
			display.setModeCalcul(MODE_CRS);
			display.setModeAffi(MODE_CRS);
		}
	}

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
			display.updateAll();
			goto piege;
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

			// maj sharp
			display.updateAll();
			goto piege;

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
		break;
	case MODE_HRM:
	case MODE_HT:
		break;

	}

	switch (display.getModeAffi()) {
	case MODE_GPS:
	case MODE_CRS:
	case MODE_PAR:
		display.updateAll();
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
		display.updateAll();
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

		display.updateAll();
		break;
	default:
		display.updateAll();
		break;

	}

	piege:
	display.updateScreen();

#ifdef __DEBUG__
	digitalWriteFast(led, HIGH);
#endif

	while (cond_wait() == 1) {

		// gather measurements
		service_peripherals();

		// sleep
		idle();
	}

}


int main () {
	setup();
	while (1) {
		loop();
	}
}

void idle() {
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	noInterrupts();
	sleep_enable();
	interrupts();
	sleep_cpu();
	sleep_disable();
}

void usage_fault_isr(void) {
	digitalWrite(led, HIGH);
	CPU_RESTART
}

void hard_fault_isr(void) {
	digitalWrite(led, HIGH);
	CPU_RESTART
}


void buttonDownEvent () {
	display.buttonDownEvent();
}

void buttonUpEvent () {
	display.buttonUpEvent();
}


void buttonPressEvent () {
	display.buttonPressEvent();
}


void buttonEvent (uint8_t evt) {
	switch (evt) {
	case 3:
		buttonPressEvent();
		break;
	case 2:
		buttonDownEvent();
		break;
	case 1:
		buttonUpEvent ();
		break;
	}
}


uint8_t cond_wait () {

	static long millis_ = millis();

	if (download_request || upload_request) {
		return 0;
	}

	if (millis() - millis_ > 1500) {
		millis_ = millis();
		return 0;
	}

	if (new_btn_data != 0) {
		millis_ = millis();
		return 0;
	}

	switch (display.getModeCalcul()) {
	case MODE_GPS:
		if (new_ancs_data != 0) {
			millis_ = millis();
			return 0;
		}
		// no break
	case MODE_CRS:
	case MODE_PAR:
		if (new_gps_data != 0 || new_gpsn_data != 0) {
			millis_ = millis();
			return 0;
		}
		break;
	case MODE_HRM:
		if (new_hrm_data != 0) {
			millis_ = millis();
			return 0;
		}
		break;
	case MODE_HT:
		if (new_cad_data != 0) {
			millis_ = millis();
			return 0;
		}
		break;
	}


	return 1;
}

