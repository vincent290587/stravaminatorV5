#include "Boucle.h"
#include "Global.h"
#include "Intonations.h"
#include "fonctions.h"
#include "fonctionsXML.h"

using namespace mvc;


void init_peripherals () {
	display.begin();
	delay(5);
	display.resetBuffer();
	display.runAffi(1);
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

	if (!SD.begin(sd_cs, SPI_HALF_SPEED))
	{
		Serial.println(F("Card initialization failed."));
		display.setSD(-1);
		display.runAffi(1);
		errorTone();
		display.setModeCalcul(MODE_HRM);
		display.setModeAffi(MODE_HRM);
		display.runAffi(1);
	}
	else {
		Serial.println(F("Card OK."));
		display.setSD(1);
		display.runAffi(1);
		delay(150);
		initListeSegments();
		Serial.print("Nombre total de segments: ");
		Serial.println(mes_segments.size());
		display.setNbSeg(mes_segments.size());
		display.runAffi(1);
		delay(1000);
		display.setModeCalcul(MODE_GPS);
		display.setModeAffi(MODE_GPS);
		display.runAffi(1);
		delay(400);
		Serial.println("Fin setup");
	}

	// needs segments and parcours to run...
	display.initSubMenus();

}


/**
 *
 */
void service_peripherals (uint8_t force) {

	static uint32_t millis_;
	static uint8_t nb_passages = 0;

	if (millis() - millis_ >= PERIPHERALS_SERVICE_TIMEOUT || force) {

		nb_passages++;
		nb_passages = nb_passages % 10;

		// save time
		millis_ = millis();

		// run display and force @ 1Hz
		display.runAffi(nb_passages == 0);

		// Power measurements
		stc.refresh();
		stc_cur.input(stc.getCurrent());

		if (nb_passages % 2 == 0) {
			// barometric meausurements
			float pressure = 0.;
			baro.getTempAndPressure(&att.temp, &pressure);
			fpressu.input(pressure);
		}

	}


}

/**
 *
 */
void gather_data() {

	// maj nordic
	if (new_btn_data) {
		new_btn_data = 0;
		display.buttonEvent(nordic.getBTN());
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


/**
 *
 */
void boucle_simu () {

	static float avance = -15.;

	Serial.println("Boucle simu");

	Serial3.println(Nordic::encodeOrder(avance, 100));

	avance += 0.5;

	if (avance > 15.) {
		avance = -15.;
	}

}

