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

	if (display.getPendingAction() != NO_ACTION) {
		display.updateScreen();
	}

	if (millis() - millis_ > PERIPHERALS_SERVICE_TIMEOUT || force) {

		// save time
		millis_ = millis();

		// backlight
		if (display.getBackLight() != 0) {
			digitalWriteFast(led, LOW);
		} else {
			digitalWriteFast(led, HIGH);
		}

		// Power measurements
		stc.refresh();
		stc_cur.input(stc.getCurrent());

		// barometric meausurements
		float pressure = 0.;
		baro.getTempAndPressure(&att.temp, &pressure);
		fpressu.input(pressure);

		//Serial.println(String("Pressure: ") + String(fpressu.output(), 2));
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

