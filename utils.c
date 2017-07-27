/* 
 * File:   utils.c
 * Author: vincent
 *
 * Created on October 27, 2015, 10:55 AM
 */


#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "utils.h"

#define FACTOR 100000.

float min(float val1, float val2) {
  if (val1 <= val2) return val1;
  else return val2;
}

float max(float val1, float val2) {
  if (val1 <= val2) return val2;
  else return val1;
}

float regFen(float val_, float b1_i, float b1_f, float b2_i, float b2_f) {
  
  float x, res;
  // calcul x
  x = (val_ - b1_i) / (b1_f - b1_i);
  
  // calcul valeur: x commun
  res = x * (b2_f - b2_i) + b2_i;
  return res;
}

float regFenLim(float val_, float b1_i, float b1_f, float b2_i, float b2_f) {
  
  float x, res;
  // calcul x
  x = (val_ - b1_i) / (b1_f - b1_i);
  
  // calcul valeur: x commun
  res = x * (b2_f - b2_i) + b2_i;
  if (res < min(b2_i,b2_f)) res = min(b2_i,b2_f);
  if (res > max(b2_i,b2_f)) res = max(b2_i,b2_f);
  return res;
}

float distance_between(float lat1, float long1, float lat2, float long2) {
  float delta = 3.141592 * (long1 - long2) / 180.;
  float sdlong = sin(delta);
  float cdlong = cos(delta);
  lat1 = 3.141592 * (lat1) / 180.;
  lat2 = 3.141592 * (lat2) / 180.;
  float slat1 = sin(lat1);
  float clat1 = cos(lat1);
  float slat2 = sin(lat2);
  float clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = delta*delta;
  delta += clat2 * sdlong * clat2 * sdlong;
  delta = sqrt(delta);
  float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return delta * 6369933;
}



void calculePos (const char *nom, float *lat, float *lon) {

    static char tab[15];
    int iLat;
    int iLon;
    
    if (!nom) {
      return;
    }

    strncpy(tab, nom, 5);
    tab[5] = '\0';
    iLat = toBase10(tab);
    if (lat) {
        *lat = (float) iLat / FACTOR - 90.;
    }

    strncpy(tab, nom + 6, 2);
    strncpy(tab + 2, nom + 9, 3);
    tab[5] = '\0';
    iLon = toBase10(tab);
    if (lon) {
        *lon = (float) iLon / FACTOR - 180.;
    }

    return;
}


long unsigned int toBase10 (char *entree) {

    static char tab[15];

    if (!entree) {
        return 0;
    }
    
    if (!strstr(entree, ".")) {
        strncpy(tab, entree, 5);
        tab[5] = '\0';
    } else {
        strncpy(tab, entree, 2);
        strncpy(tab + 2, entree + 3, 3);
        tab[5] = '\0';
    }

    return strtoul(tab, NULL, 36);

}


int percentageBatt (float tensionValue) {

  float fp_ = 0.;

  if (tensionValue > 3.78) {
    fp_ = 536.34*tensionValue*tensionValue*tensionValue-6723.8*tensionValue*tensionValue;
    fp_ += 28186*tensionValue-39402;

    if (fp_ > 100.) fp_ = 100.;
    
  } else if (tensionValue > 2.) {
    fp_ = pow(10, -11.4)*pow(tensionValue, 22.315);
  } else {
    fp_ = 0;
  }

  return (int)fp_;
}
