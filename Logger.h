
#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>


#ifdef __SST__
void resetWhiteBox();

void checkSST26();

void setToBeErased ();

uint32_t getLastAddress();

void incrementeAddress ();

void dumpLogGPS();
#endif

void loggerMsg(const char *msg_);

void loggerMsg(int val_);

void loggerMsg(float val_1, float val_2);

void loggerHT();

void loggerRR();

void loggerData();

void ecrireHeader ();

void effacerHisto();

#endif
