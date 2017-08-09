/*

  File:   Merites.cpp
   Author: vincent

   Created on September 11, 2014, 11:01 AM
*/

#include "Global.h"
#include "Merites.h"
#include <WProgram.h>




Merite::Merite() {
  distance = 0.;
  climb = 0.;
  last_stored_ele = 0.;
  last_stored_lat = -100.;
  last_stored_lon = -200.;
  pr_won = 0;
  puissance = 0.;
  vit_asc = 0;
}

// GETTERS

float Merite::getClimb() {
  return climb;
}

float Merite::getDistance() {
  return distance;
}

float Merite::getPower() {
  return puissance;
}



// Mise a jour avec la pos GPS courante
int Merite::majMerite(float lat, float lon, float ele) {

  static float majDist = 0;
  static int res;

  res = 0;

  if (last_stored_lat < -90.) {
    // initialisation
    last_stored_lat = lat;
    last_stored_lon = lon;
    last_stored_ele = ele;
    distance = 0.;
    climb = 0.;
  }

  majDist = distance_between(lat, lon, last_stored_lat, last_stored_lon);

  // mise a jour de la distance parcourue
  // tous les X metres
  if (majDist > DIST_RECORD) {
    distance += majDist;
    last_stored_lat = lat;
    last_stored_lon = lon;
    res = 1;
  }

  // hysteresis
  if (res == 1) {
    if (ele > last_stored_ele + 1.) {
      // mise a jour de la montee totale
      climb += ele - last_stored_ele;
      last_stored_ele = ele;
    }
    else if (ele + 1. < last_stored_ele) {
      // on descend, donc on garde la derniere alti
      // la plus basse
      last_stored_ele = ele;
    }
  }

  return res;
}

/**
 *
 */
void Merite::resetClimb(float new_ele) {

	last_stored_ele = new_ele;
	climb = 0.;

}

/**
 *
 */
void Merite::majPower(ListePoints *mes_points, float speed_) {

  static float fSpeed = -1.;
  Point P1, P2, Pc;
  float dTime;
  uint8_t i;

  if (!mes_points) return;

  if (mes_points->longueur() <= FILTRE_NB + 1) return;

  P1 = mes_points->getFirstPoint();
  P2 = mes_points->getPointAt(FILTRE_NB);

  dTime = P1._rtime - P2._rtime;

  if (fabs(dTime) > 1.5 && fabs(dTime) < 25) {

    // calcul de la vitesse ascentionnelle par regression lineaire
    for (i = 0; i <= FILTRE_NB; i++) {
      Pc = mes_points->getPointAt(i);
      _x[i] = Pc._rtime - P2._rtime;
      _y[i] = Pc._alt;
      Serial.println("Alt " + String(_x[i], 2) + " / " + String(_y[i], 1));
    }

    _lrCoef[1] = _lrCoef[0] = 0;
    this->simpLinReg(_x, _y, _lrCoef, FILTRE_NB + 1);

    // STEP 1 : on filtre altitude et vitesse
    vit_asc = _lrCoef[0];
    //Serial.println("testVit= " + String(vit_asc, 2));

    // horizontal speed (m/s)
    fSpeed = speed_ / 3.6;

    // STEP 2 : Calcul
    puissance = 9.81 * MASSE * vit_asc; // grav
    puissance += 0.004 * 9.81 * MASSE * fSpeed; // sol + meca
    puissance += 0.204 * fSpeed * fSpeed * fSpeed; // air
    puissance *= 1.025; // transmission (rendement velo)

    if (fSpeed < 1.5) {
      puissance = -100.;
    }
  } else {
	  Serial.println("dTime= " + String(dTime, 2));
  }

  return;
}


void Merite::simpLinReg(float* x, float* y, float* lrCoef, int n) {
  // pass x and y arrays (pointers), lrCoef pointer, and n.  The lrCoef array is comprised of the slope=lrCoef[0] and intercept=lrCoef[1].  n is length of the x and y arrays.
  // http://en.wikipedia.org/wiki/Simple_linear_regression

  // initialize variables
  float xbar = 0;
  float ybar = 0;
  float xybar = 0;
  float xsqbar = 0;

  // calculations required for linear regression
  for (int i = 0; i < n; i++) {
    xbar = xbar + x[i];
    ybar = ybar + y[i];
    xybar = xybar + x[i] * y[i];
    xsqbar = xsqbar + x[i] * x[i];
  }
  xbar = xbar / n;
  ybar = ybar / n;
  xybar = xybar / n;
  xsqbar = xsqbar / n;

  // simple linear regression algorithm
  lrCoef[0] = (xybar - xbar * ybar) / (xsqbar - xbar * xbar);
  lrCoef[1] = ybar - lrCoef[0] * xbar;
}

