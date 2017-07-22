#include <TinyGPS++.h>
#include <Adafruit_GFX.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <AltiBaro.h>
#include <Adafruit_GPS.h>
#include <STC3100.h>
#include <list>
#include <avr/sleep.h>
#include <Nordic.h>
#include <IntelliScreen.h>
#include <SerialFlash.h>
#include "TSharpMem.h"
#include "TLCD.h"
#include "Merites.h"
#include "ListePoints.h"
#include "Segment.h"
#include "Parcours.h"
#include "define.h"


TLCD display(sharp_cs);

TinyGPSPlus gps;
TinyGPSCustom hdop(gps, "GPGSA", 16); // $GPGSA sentence, 16th element
TinyGPSCustom vdop(gps, "GPGSA", 17); // $GPGSA sentence, 17th element

Nordic nordic;

Merite cumuls;

#ifdef __SST__
SerialFlashFile sst;
#endif

AltiBaro baro = AltiBaro();

STC3100 stc = STC3100(0x70);

Adafruit_GPS pmkt(&Serial1);

void setup() {
  att.has_started = 0;
  // initialize the digital pin as an output.
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial3.begin(115200);
  Serial3.attachRts(2);
  Serial3.attachCts(14);
  pinMode(led, OUTPUT);
  digitalWriteFast(led, HIGH);

  // pseudo boutons
  pinMode(virtbtn0, OUTPUT);
  digitalWriteFast(virtbtn0, HIGH);
  pinMode(virtbtn1, OUTPUT);
  digitalWriteFast(virtbtn1, HIGH);

  memset(&att, 0, sizeof(SAttitude));

  delay(50);
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

  // init GPS
  ////pmkt.sendCommand("$PMTK103*30"); // cold start
  delay(1000);
  pmkt.sendCommand("$PMTK314,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29");


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

  if (!SD.begin(sd_cs)) {
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
  att.has_started = 1;
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
          // demande de mise a jour des segments
          mode_simu = 1;
          Serial.println("Mode simu activé");
        }
      }
    } else {
      if (c == '#') {
        Serial.println("Mode simu desactivé");
        mode_simu = 0;
      }
      if (gps.encode(c)) {
        new_gps_data = 1;
      }
    }
  }
}

void serialEvent1() {
  char c;
  while (Serial1.available() && att.has_started > 0) {
    c = Serial1.read();
#ifdef __DEBUG_GPS__
    Serial.write(c);
#endif
    if (mode_simu == 0) {
      bool enc = gps.encode(c);
      if (enc && millis() - lastFix > 500) {
        if (gps.location.isValid()) {
          lastFix = millis();
          Serial.println("Loc OK");
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
      Serial.println(String("Format: ") + _format);
    }
#endif
    switch (_format) {
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

// the loop routine runs over and over again forever:
void loop() {
#ifdef __DEBUG__
  digitalWriteFast(led, LOW);
  Serial.println("LOOP");

  Serial.println(String("Satellites in view: ") + gps.satellites.value());
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
  if (new_cad_data && nordic.getRPM()) att.cad_rpm = nordic.getRPM();
  if (new_cad_data && nordic.getSpeed() > 0.) att.cad_speed = nordic.getSpeed();
  if (new_ancs_data) {
    if (alertes_nb < 20) {
      alertes_nb += 1;
    }
    String tmp = String ("") + nordic.getANCS_title() + nordic.getANCS_msg();
    if (tmp.indexOf("ppel") > 0 || tmp.indexOf("essage") > 0) basicTone();
    display.notifyANCS(nordic.getANCS_type(), nordic.getANCS_title(), nordic.getANCS_msg());
  }
  if (new_dbg_data) {
    String tmp = String("Erreur nRF51: code=") + String(nordic.getDBG_code(), HEX) + " - line:" + String(nordic.getDBG_line());
    loggerMsg(tmp.c_str());
    if (nordic.getDBG_type() == 1) loggerMsg(nordic.getDBG_msg());
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


  // backlight
  if (display.getBackLight() != 0) {
    digitalWriteFast(led, LOW);
  } else {
    digitalWriteFast(led, HIGH);
  }


  // Get a new STC measurement
  stc.getBatteryData(&batt_data);
  att.cbatt = batt_data.Current;
  att.pbatt = percentageBatt(batt_data.Voltage);
  att.vbatt = batt_data.Voltage;


#ifdef __DEBUG_STC__
  Serial.print("Temperature STC :    ");
  Serial.print(batt_data.Temperature, 2);
  Serial.println(" degres C");
  Serial.print("Current STC     :    ");
  Serial.print(batt_data.Current, 2);
  Serial.println(" mA");
  Serial.print("Voltage STC     :    ");
  Serial.print(batt_data.Voltage, 2);
  Serial.println(" V");
  Serial.print("Charge STC      :    ");
  Serial.print(batt_data.Charge, 2);
  Serial.println(" mAh");
#endif

  new_gps_data = 0;
  new_hrm_data = 0;
  new_cad_data = 0;
  new_ancs_data = 0;
  new_btn_data = 0;
  new_dbg_data = 0;
  download_request = 0;
  upload_request = 0;

  // si pas de fix on affiche la page d'info GPS
  if (display.getModeCalcul() != MODE_HRM && display.getModeCalcul() != MODE_HT) {
    if (att.nbpts > MIN_POINTS + 10 && !gps.location.isValid() && display.getModeAffi() != MODE_GPS) {
      pmkt.sendCommand("$PMTK314,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29");
      display.setStoredMode(display.getModeAffi());
      display.setModeAffi(MODE_GPS);
    } else if (att.nbpts > MIN_POINTS + 10 && gps.location.isValid() && display.getModeAffi() == MODE_GPS) {
      pmkt.sendCommand("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28");
      display.setModeAffi(display.getStoredMode());
    }
  }

  // aiguillage pour chaque type d'affichage
  switch (display.getModeCalcul()) {
    case MODE_GPS:
      display.setNbSatIV(0);
      display.setNbSatU((uint16_t)gps.satellites.value());
      display.setHDOP(String(hdop.value()).toInt());

    case MODE_CRS:
      // recup infos gps
      att.lat = gps.location.lat();
      att.lon = gps.location.lng();
      age     = gps.location.age();
      
      if (!gps.location.isValid()) goto piege;
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

      // maj BMP
      updateAltitude(&att.alt);

      mes_points.enregistrePos(att.lat, att.lon, att.alt, att.secj);

      // maj merites
      if (att.nbpts < MIN_POINTS) {
        // maj sharp
        display.updateAll(&att);

        if (att.nbpts < 4) {
          basicTone();
        }
        goto piege;
      } else if (att.nbpts == MIN_POINTS) {
        // maj GPS
        pmkt.sendCommand("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28");
        // maj BMP
        updateAltitudeOffset(&att.alt);
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
      } else if (att.speed < 3.) {
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
      display.updateAll(&att);
      break;
    case MODE_SIMU:
      boucle_simu();
    case MODE_HRM:
    case MODE_HT:
      display.updateAll(&att);
      break;

  }

piege:
  display.updateScreen();

#ifdef __DEBUG__
  digitalWriteFast(led, HIGH);
#endif

  while (cond_wait() == 1) {
    idle();
  }

}

void idle() {
  set_sleep_mode(SLEEP_MODE_IDLE);
  noInterrupts();
  sleep_enable();
  interrupts();
  asm("wfi");
  sleep_disable();
  yield();
}

void usage_fault_isr(void) {
  digitalWrite(led, HIGH);
  CPU_RESTART
}

void software_isr(void) {
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
      buttonUpEvent ();
      break;
    case 1:
      buttonDownEvent();
      break;
  }
}

void activerNavigateur() {
  return;
}

void desactiverNavigateur() {
  return;
}


