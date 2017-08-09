/*
 * BoucleCRS.cpp
 *
 *  Created on: 9 août 2017
 *      Author: Vincent
 */


#include "Global.h"
#include "fonctions.h"
#include "Intonations.h"
#include "fonctionsXML.h"
#include "BoucleCRS.h"

using namespace mvc;


void boucle_outdoor () {

	float min_dist_seg = 50000;
	float tmp_dist;
	uint8_t order_glasses;

	resetdelay_();

	if (att.cad_speed > CAD_SPD_PW_LIM && att.cad_rpm > 0 && !mode_simu) {
		// speed from CAD
		cumuls.majPower(&mes_points, att.cad_speed);
	} else {
		// speed from GPS
		cumuls.majPower(&mes_points, att.speed);
	}
	att.power = cumuls.getPower();

	std::list<Segment>::iterator _iter;
	Segment *seg;

	att.nbact = 0;
	order_glasses = 0;
	display.resetSegments();

	for (_iter = mes_segments._segs.begin(); mes_segments.size() != 0 && _iter != mes_segments._segs.end(); _iter++) {

		seg = _iter.operator->();

		if (seg->isValid() && time_c < 930) {

			tmp_dist = watchdog (seg, att.lat, att.lon);

			if (tmp_dist < min_dist_seg && seg->getStatus() == SEG_OFF) min_dist_seg = tmp_dist;

			seg->majPerformance(&mes_points);

			if (seg->getStatus() != SEG_OFF) {
				att.nbact += 1;
				if (seg->getStatus() == SEG_START) {
					Serial.println("Segment commence");
					segStartTone ();
				} else if (seg->getStatus() == SEG_FIN) {
					Serial.println("Segment termine");
					if (seg->getAvance() > 0.) {
						att.nbpr++;
						att.nbkom++;
						victoryTone ();
					}
				} else if (seg->getStatus() == SEG_ON && order_glasses == 0) {
					Serial3.println(Nordic::encodeOrder(seg->getAvance(), seg->getCur()));
					order_glasses = 1;
				}
				display.registerSegment(seg);
			} else if (tmp_dist < SEG_DISPLAY_DIST) {
				// just display the segment
				display.registerSegment(seg);
			}

		}

		// pour taches de fond
		delayMicroseconds(10);

	} // fin for

	// information to glasses
	if (order_glasses == 0) {
		Serial3.println(Nordic::encodeOrder(5., 5.));
		order_glasses = 1;
	}

	att.next = min_dist_seg;

	time_c = millis() - start;

#ifdef __DEBUG__
	Serial.println(String(F("Next Seg: ")) + min_dist_seg + F("  /  Nb seg actifs: ") + att.nbact);
	Serial.println(String(F("Temps de la boucle:  ")) + time_c + F("ms"));
#endif
}


uint8_t updateLocData() {

	uint8_t res = 1;

	if (new_gps_data && mode_simu == 0) {
		if (gps.location.isUpdated()) {

			// recup infos gps
			att.lat = gps.location.lat();
			att.lon = gps.location.lng();
			age     = gps.location.age();

			att.nbpts++;

			if (gps.altitude.meters() < 5000.) {
				att.gpsalt = gps.altitude.meters();
			} else if (att.nbpts <= MIN_POINTS) {
				att.gpsalt = 0.;
			} else {
				att.gpsalt = att.alt;
			}

			att.speed = gps.speed.kmph();

			att.secj = get_sec_jour();

			att.gps_src = 0;

			last_true_gps = millis();

#ifdef __DEBUG_GPS__
			Serial.println(F("Using true GPS data"));
#endif

			res = 0;
		}
	} else if (new_gpsn_data && (millis() - last_true_gps > 2500)) {
		att.lat  = nordic.getLat() / 10000000.;
		att.lon  = nordic.getLon() / 10000000.;
		att.gpsalt = nordic.getEle();
		att.speed = (float)nordic.getGpsSpeed() / 100.;

		age      = 0;

		att.nbpts++;

		att.secj = nordic.getSecJ();

		last_nrf_gps = millis();

		att.gps_src = 1;

		res = 0;

#ifdef __DEBUG_GPS__
		Serial.println(F("Using Nordic GPS data"));
#endif

	}

	// reset flags
	new_gps_data  = 0;
	new_gpsn_data = 0;

	return res;
}

uint8_t isLocOutdated() {

	if (millis() - last_true_gps < 5000) return 0;
	if (millis() - last_nrf_gps < 5000) return 0;

	return 1;
}



void updateAltitudeOffset () {

	float pressure;
	// pressure est mis a jour ici
	baro.getTempAndPressure(&att.temp, &pressure);
	// mise a jour de la pression au niveau de la mer
	baro.seaLevelForAltitude(att.gpsalt, pressure);
	// recalcul avec la premiere mise a jour
	float corr_alt = baro.pressureToAltitude(fpressu.output());
	// reset merites
	cumuls.resetClimb(corr_alt);
}



