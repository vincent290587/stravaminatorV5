#include <SPI.h>
#include <i2c_t3.h>

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

IntervalTimer myTimer;


void stop_power_save(void) {
	loop_save_power = 0;
}

void idle() {
	noInterrupts();
	sleep_cpu();
	interrupts();
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

void setup() {
	memset(&att, 0, sizeof(SAttitude));
	att.has_started = 0;

	Serial.begin(115200);
	Serial3.begin(115200);
	//Serial3.attachRts(2);
	//Serial3.attachCts(14);
	pinMode(led, OUTPUT);
#ifdef __DEBUG__
	digitalWriteFast(led, HIGH);
#endif

	// snooze timer in milliseconds
	timer.setTimer(25);

	/********************************************************
	     Define digital pins for waking the teensy up. This
	     combines pinMode and attachInterrupt in one function.
	 ********************************************************/
//	digital.pinMode(0, INPUT_PULLUP, FALLING);//pin, mode, type
//	digital.pinMode(7, INPUT_PULLUP, FALLING);//pin, mode, type

	is_gps_awake = 1;

	att.cad_speed = 0;
	is_cad_init = 0;
	last_nrf_cad = millis();

	// pseudo boutons
	pinMode(virtbtn0, OUTPUT);
	digitalWriteFast(virtbtn0, HIGH);
	pinMode(virtbtn1, OUTPUT);
	digitalWriteFast(virtbtn1, HIGH);

	// init all the peripherals
	init_peripherals();

	// Initialize all the uninitialized TinyGPSCustom objects
	for (int i = 0; i < 4; ++i)
	{
		satNumber[i].begin(gps, "GPGSV", 4 + 4 * i); // offsets 4, 8, 12, 16
		snr[i].begin(gps, "GPGSV", 7 + 4 * i); // offsets 7, 11, 15, 19
	}

	// Setup for Master mode, pins 18/19, external pullups, 100kHz
	Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);

	// start listening to GPS
	Serial1.begin(9600);

	service_peripherals(1);

	att.has_started = 1;

	myTimer.begin(stop_power_save, 50000);

#ifdef __DEBUG__
	digitalWriteFast(led, LOW);
#endif
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

	gather_data();

	display.runCalcul();

#ifdef __DEBUG__
	digitalWriteFast(led, HIGH);
#endif

	while (cond_wait()) {

		mtpd.loop();

		// gather measurements
		service_peripherals();

		// child loop more efficient
		loop_save_power = 0;
		do {
			// TODO sleep
			Snooze.idle(config_teensy35);
//			idle();
		} while (loop_save_power);

	}

}

int main () {
	setup();
	while (1) {
		mtpd.loop();
		loop();
	}
}

void usage_fault_isr(void) {
	digitalWrite(led, HIGH);
	CPU_RESTART
}

void hard_fault_isr(void) {
	digitalWrite(led, HIGH);
	CPU_RESTART
}

