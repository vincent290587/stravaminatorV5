#include "Boucle.h"
#include "Global.h"
#include "Intonations.h"
#include "fonctions.h"
#include "fonctionsXML.h"

using namespace mvc;

uint8_t cond_wait () {

  static long millis_ = millis();

  if (download_request || upload_request) {
    return 0;
  }

  if (display.getPendingAction() != NO_ACTION) {
    display.updateScreen();
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

void boucle_simu () {

    static float avance = -15.;

    Serial.println("Boucle simu");

    Serial3.println(Nordic::encodeOrder(avance, 100));

    avance += 0.5;

    if (avance > 15.) {
      avance = -15.;
    }
  
}

