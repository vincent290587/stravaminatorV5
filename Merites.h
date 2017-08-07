/*
   File:   Merites.hpp
   Author: vincent

   Created on September 11, 2014, 9:25 AM
*/

#ifndef MERITES_HPP
#define	MERITES_HPP

#include "ListePoints.h"
#include "Filter.h"

#define DIST_RECORD 10.

#define FILTRE_NB   6

#define MASSE       79.

//////// filter coefficients
const float vspeed_coefficients[10] =
{
		// Scaled for floating point

		0.053805042058918236, 0.10761008411783647, 0.053805042058918236, 1.1295059120217161, -0.33775737827236,// b0, b1, b2, a1, a2
		0.0625, 0.125, 0.0625, 1.4013136354923321, -0.6596793652882533// b0, b1, b2, a1, a2

};

class Merite {

  public:
    Merite();
    int majMerite(float lat, float lon, float ele);
    float getDistance();
    float getClimb();
    float getPower();
    float getVitAsc() {return vit_asc;}
    void majPower(ListePoints *mes_points, float speed_);

  private:
    float distance;
    float climb;
    float puissance;
    float last_stored_ele;
    float last_stored_lat;
    float last_stored_lon;
    int pr_won;
    float _y[FILTRE_NB+1];
    float _x[FILTRE_NB+1];
    float _lrCoef[2];
    float vit_asc;

    void simpLinReg(float* x, float* y, float* lrCoef, int n);
};



#endif	/* MERITES_HPP */



