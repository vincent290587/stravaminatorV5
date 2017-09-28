#include "Boucle.h"
#include "Global.h"
#include "Intonations.h"
#include "fonctions.h"
#include "fonctionsXML.h"

using namespace mvc;

/**
 *
 */
void service_peripherals (uint8_t force) {

	static uint32_t millis_;
	static uint8_t nb_passages = 0;

	if (millis() - millis_ > PERIPHERALS_SERVICE_TIMEOUT || force) {

		nb_passages++;
		nb_passages = nb_passages % 10;

		// save time
		millis_ = millis();

		// run display and force @ 1Hz
		display.run(nb_passages == 0);

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


void boucle_simu () {

	static float avance = -15.;

	Serial.println("Boucle simu");

	Serial3.println(Nordic::encodeOrder(avance, 100));

	avance += 0.5;

	if (avance > 15.) {
		avance = -15.;
	}

}

