

#ifndef VUE_GLOBAL_H_
#define VUE_GLOBAL_H_


#include <Adafruit_GFX.h>
#include <SdFat.h>
#include <AltiBaro.h>
#include "myGPS.h"
#include <STC3100.h>
#include <Nordic.h>
#include <IntelliScreen.h>
#include <SerialFlash.h>
#include "TSharpMem.h"
#include "TLCD.h"
#include "Merites.h"
#include "ListePoints.h"
#include "Segment.h"
#include "Parcours.h"
#include "Filter.h"

#include "define.h"

#define BATT_INT_RES                   0.1
#define CAD_SPEED_TIMEOUT_MS           3000


namespace mvc {

// set up variables using the SD utility library functions:
extern SdFat sd;
extern SdFile file;
extern SdFile gpx;

extern TLCD display;

extern myGPS gps;
extern TinyGPSCustom hdop; // $GPGSA sentence, 16th element
extern TinyGPSCustom vdop; // $GPGSA sentence, 17th element
extern TinyGPSCustom satsInView;         // $GPGSV sentence, third element
extern TinyGPSCustom satNumber[4];
extern TinyGPSCustom snr[4];

extern Nordic nordic;

extern Merite cumuls;

#ifdef __SST__
extern SerialFlashFile sst;
#endif

extern AltiBaro baro;

extern STC3100 stc;

extern SAttitude att;

extern ListeSegments mes_segments;

extern ListePoints   mes_points;

extern ListeParcours mes_parcours;

extern Filter hspeed;
extern Filter stc_cur;
extern Filter fpressu;

extern const uint8_t virtbtn0; // PTD0
extern const uint8_t virtbtn1; // PTD1
extern const uint8_t led;
extern const uint8_t sd_cs;// PTC1

extern const uint8_t sharp_cs;

extern const uint8_t memCs;
extern const uint8_t memWp;
extern const uint8_t memHold;

extern unsigned long age;

extern unsigned long start;
extern unsigned long time_c;
extern unsigned long lastFix;

extern uint32_t last_true_gps;
extern uint32_t last_nrf_gps;
extern uint32_t last_nrf_cad;

extern uint8_t new_gps_data;
extern uint8_t new_gpsn_data;
extern uint8_t new_hrm_data;
extern uint8_t new_cad_data;
extern uint8_t new_ancs_data;
extern uint8_t new_btn_data;
extern uint8_t new_dbg_data;
extern uint8_t download_request;
extern uint8_t upload_request;
extern uint8_t mode_simu;
extern uint8_t alertes_nb;
}


class Global {
public:
	Global();
};

#endif /* VUE_GLOBAL_H_ */
