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

#define FILTRE_CAD_ACC   1.6

#define MASSE       79.


class Merite {

  public:
    Merite();
    int majMerite(float lat, float lon, float ele);
    float getDistance();
    float getClimb();
    float getPower();
    float getVitAsc() {return vit_asc;}
    void majPower(ListePoints *mes_points, float speed_);
    int  majCRS(float cad_speed, int8_t hrm, uint16_t power);

    void resetClimb(float new_ele);

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

    float m_cad_speed_prev;
    float m_ht_last_update_time;

    void simpLinReg(float* x, float* y, float* lrCoef, int n);
};



#endif	/* MERITES_HPP */



