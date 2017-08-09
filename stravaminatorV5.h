
#ifndef __STRAVA5_H__
#define __STRAVA5_H__


void setup();

// the loop routine runs over and over again forever:
void loop();

void idle();

uint8_t cond_wait ();

void usage_fault_isr(void);

void software_isr(void);

void hard_fault_isr(void);

void buttonDownEvent ();

void buttonUpEvent ();

void buttonPressEvent ();

void buttonEvent (uint8_t evt);

#endif
