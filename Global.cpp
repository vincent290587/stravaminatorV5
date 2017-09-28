/*
 * Global.cpp
 *
 *  Created on: 9 avr. 2017
 *      Author: Vincent
 */

#include "Global.h"


//////// filter coefficients
const float vspeed_coefficients[10] =
{
		// Scaled for floating point

		0.053805042058918236, 0.10761008411783647, 0.053805042058918236, 1.1295059120217161, -0.33775737827236,// b0, b1, b2, a1, a2
		0.0625, 0.125, 0.0625, 1.4013136354923321, -0.6596793652882533// b0, b1, b2, a1, a2

};


namespace mvc {

MTPStorage_SD storage;
MTPD          mtpd(&storage);

//SnoozeDigital digital;
SnoozeTimer timer;
SnoozeUSBSerial usb;

SnoozeBlock config_teensy35(usb, timer);

// set up variables using the SD utility library functions:
SdFile file;
SdFile gpx;

DisplayManager display;

myGPS         gps(&Serial1);
TinyGPSCustom hdop(gps, "GPGSA", 16); // $GPGSA sentence, 16th element
TinyGPSCustom vdop(gps, "GPGSA", 17); // $GPGSA sentence, 17th element
TinyGPSCustom satsInView(gps, "GPGSV", 3);         // $GPGSV sentence, third element
TinyGPSCustom satNumber[4];
TinyGPSCustom snr[4];

Nordic nordic;

Merite cumuls;

#ifdef __SST__
SerialFlashFile sst;
#endif

AltiBaro baro = AltiBaro();

STC3100 stc = STC3100(0x70);

SAttitude att;

ListeSegments mes_segments;

ListePoints   mes_points;

ListeParcours mes_parcours;

// filters
Filter hspeed(vspeed_coefficients);
Filter stc_cur(vspeed_coefficients);
Filter fpressu(vspeed_coefficients);

const uint8_t virtbtn0 = 2; // PTD0
const uint8_t virtbtn1 = 14; // PTD1
const uint8_t led = 17;
const uint8_t sd_cs = 22;// PTC1

const uint8_t sharp_cs = 23;

const uint8_t memCs = 15; //chip select      PTC0
const uint8_t memWp = 44; //write protection PTB23
const uint8_t memHold = 45; //hold           PTB22

unsigned long age;

unsigned long start;
unsigned long time_c = 0;
unsigned long lastFix = 0;

uint32_t last_true_gps = 0;
uint32_t last_nrf_gps = 0;
uint32_t last_nrf_cad;

uint8_t loop_save_power = 0;
uint8_t new_gps_data = 0;
uint8_t new_gpsn_data = 0;
uint8_t new_hrm_data = 0;
uint8_t new_cad_data = 0;
uint8_t new_ancs_data = 0;
uint8_t new_btn_data = 0;
uint8_t new_dbg_data = 0;
uint8_t download_request = 0;
uint8_t upload_request = 0;
uint8_t mode_simu = 0;
uint8_t alertes_nb = 0;
uint8_t is_cad_init = 0;
uint8_t is_gps_awake = 1;
}

Global::Global() {
	// TODO Auto-generated constructor stub

}

