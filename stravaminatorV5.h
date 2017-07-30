
#ifndef __STRAVA5_H__
#define __STRAVA5_H__


void setup();

// the loop routine runs over and over again forever:
void loop();

void idle();

void usage_fault_isr(void);

void software_isr(void);

void hard_fault_isr(void);

void buttonDownEvent ();

void buttonUpEvent ();

void buttonPressEvent ();

void buttonEvent (uint8_t evt);

void activerNavigateur();

void desactiverNavigateur();

uint8_t updateLocData();

uint8_t isLocOutdated();


void updateAltitude(float *_alt);

void updateAltitudeOffset (float *_alt);

#endif
