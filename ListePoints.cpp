/* 
 * File:   ListePoints.cpp
 * Author: vincent
 * 
 * Created on October 15, 2015, 3:30 PM
 */

#include "ListePoints.h"
#include "utils.h"
#include <math.h>

Point2D::Point2D() {
    _lat = 0;
    _lon = 0;
}

Point2D::Point2D(float lat, float lon) {
    _lat = lat;
    _lon = lon;
}

Point2D & Point2D::operator=(const Point2D *point) {
    if (point != 0) {
        _lat = point->_lat;
        _lon = point->_lon;
        return *this;
    } else {
        return *this;
    }
}

Point2D & Point2D::operator=(const Point2D &point) {
    _lat = point._lat;
    _lon = point._lon;
    return *this;
}

Point::Point() {
    _lat = 0;
    _lon = 0;
    _alt = 0;
    _rtime = 0;
}

Point::Point(float lat, float lon, float alt, float rtime) {
    _lat = lat;
    _lon = lon;
    _alt = alt;
    _rtime = rtime;
}


int Point::isValid() {
    if (_lat == 0. || _lon == 0. || abs(_lat) > 89. || abs(_lon) > 189.) {
        return 0;
    } else {
        return 1;
    }
}


Point & Point::operator=(const Point *point) {
    if (point != 0) {
        _lat = point->_lat;
        _lon = point->_lon;
        _alt = point->_alt;
        _rtime = point->_rtime;
        return *this;
    } else {
        return *this;
    }
}

Point & Point::operator=(const Point &point) {
    _lat = point._lat;
    _lon = point._lon;
    _alt = point._alt;
    _rtime = point._rtime;
    return *this;
}

void Point::toString() {
    //printf("Point -> %f %f %f %f\n", _lat, _lon, _alt, _rtime);
}

ListePoints::ListePoints() {
}

void ListePoints::ajouteDebut(float lat, float lon, float alt, float msec) {
    _lpoints.push_front(Point(lat, lon, alt, msec));
}

void ListePoints::ajouteFin(float lat, float lon, float alt, float msec) {
    _lpoints.push_back(Point(lat, lon, alt, msec));
}

void ListePoints::enregistrePos(float lat, float lon, float alt, float msec) {
    _lpoints.push_front(Point(lat, lon, alt, msec));
    if (_lpoints.size() > NB_RECORDING) {
      _lpoints.pop_back();
    }
}

int ListePoints::longueur() {
    return _lpoints.size();
}

void ListePoints::vider() {
    _lpoints.clear();
    return;
}

Point *ListePoints::getFirstPoint() {
    return &_lpoints.front();
}

Point *ListePoints::getLastPoint() {
    return &_lpoints.back();
}

Point *ListePoints::getPointAt(int i) {
    std::list<Point>::iterator _iter;
    int j;

    if (i >= this->longueur() || (i < 0 && 1 - i < 0)) {
        return 0;
    }

    if (i < 0) {
        // -1 est le dernier point
        _iter = _lpoints.end();
        _iter--;
        for (j=-1; j > i; j--) {
          _iter--;
        }
    } else {
        _iter = _lpoints.begin();
        for (j=1; j <= i; j++) {
          _iter++;
        }
    }

    return _iter.operator->();
}

void ListePoints::supprLast() {
    _lpoints.pop_back();
}

void ListePoints::toString() {
    std::list<Point>::iterator _iter;

    if (this->longueur() > 0) {
        for (_iter = _lpoints.begin(); _iter != _lpoints.end(); _iter++) {

            _iter.operator->()->toString();

        }
    } else {
        //printf("Aucun point\n");
    }

}

float ListePoints::dist(Point *p_) {

  float maDist = 100000.;
  Point pCourant;
  std::list<Point>::iterator _iter;
  
  // on cherche la taille de fenetre
  for (_iter = _lpoints.begin(); _iter != _lpoints.end(); _iter++) {

    pCourant = *_iter.operator->();

    if (maDist > pCourant.dist(p_)) {
      maDist = pCourant.dist(p_);
    }

  }
  return maDist;
}

float ListePoints::dist(float lat_, float lon_) {

  float maDist = 100000.;
  Point pCourant, p_;
  std::list<Point>::iterator _iter;
  
  p_._lat = lat_;
  p_._lon = lon_;
  
  // on cherche la taille de fenetre
  for (_iter = _lpoints.begin(); _iter != _lpoints.end(); _iter++) {

    pCourant = *_iter.operator->();

    if (maDist > pCourant.dist(p_)) {
      maDist = pCourant.dist(p_);
    }

  }
  return maDist;
}


Vecteur ListePoints::posRelative(Point point) {

    std::list<Point>::iterator _iter;
    Point P1, P2, tmpPT;
    float tmp_dist, distP1, distP2;
    int init = 0;
    Vecteur res, PP1, P1P2;

    if (_lpoints.size() < 5) {
        return res;
    }

    // on cherche les deux plus proches points
    for (_iter = _lpoints.begin(); _iter != _lpoints.end(); _iter++) {

        tmpPT = _iter.operator->();
        tmp_dist = tmpPT.dist(&point);

        if (init == 0) {
            P1 = tmpPT;
            distP1 = P1.dist(&point);
            init++;
        } else if (init == 1) {
            P2 = tmpPT;
            distP2 = P2.dist(&point);
            init++;
        } else {

            if (tmp_dist < distP1) {
                if (distP1 < distP2) {
                    P2 = P1;
                    distP2 = P2.dist(&point);
                }
                P1 = tmpPT;
                distP1 = P1.dist(&point);

            } else if (tmp_dist < distP2) {
                P2 = tmpPT;
                distP2 = P2.dist(&point);
            }
        }

    }

    // construction des vecteurs
    PP1 = Vecteur(point, P1);
    P1P2 = Vecteur(P2, P1);

    tmp_dist = P1.dist(&P2);

    distP1 = P1.dist(&point);

    // interpolation
    if (tmp_dist > 0.000001) {
        res._y = (PP1._x * P1P2._x + PP1._y * P1P2._y) / tmp_dist;
        res._x = sqrt(distP1 * distP1 - res._y * res._y);
        res._z = P1._alt + (P2._alt - P1._alt) * res._y / tmp_dist;
        res._t = P1._rtime + (P2._rtime - P1._rtime) * res._y / tmp_dist;
    } else {
        res._x = P1._lon;
        res._y = P1._lat;
        res._z = P1._alt;
        res._t = P1._rtime;
        //loggerMsg("Distance inter-point trop faible !!");
    }

    return res;
}



// difference d'elevation d'une liste de points

Vecteur ListePoints::deltaListe() {

    Vecteur mon_delta;
    std::list<Point>::iterator _iter;
    Point *tmpPT1, *tmpPT2;
    float tmp_dist = 0.;
    float min_lon = _lpoints.front()._lon;
    float max_lon = _lpoints.front()._lon;
    float min_lat = _lpoints.front()._lat;
    float max_lat = _lpoints.front()._lat;
    
    
    for (_iter = _lpoints.begin(); _iter != _lpoints.end();) {

        tmpPT1 = _iter.operator->();
        _iter++;
        if (_iter == _lpoints.end()) break;
        tmpPT2 = _iter.operator->();
        
        // distance
        tmp_dist += tmpPT1->dist(tmpPT2);
        
        // minimaux
        if (tmpPT2->_lon < min_lon) min_lon = tmpPT2->_lon;
        if (tmpPT2->_lat < min_lat) min_lat = tmpPT2->_lat;
        
        // maximaux
        if (max_lon < tmpPT2->_lon) max_lon = tmpPT2->_lon;
        if (max_lat < tmpPT2->_lat) max_lon = tmpPT2->_lat;

    }
    
    mon_delta._x = max_lon - min_lon;
    mon_delta._y = max_lat - min_lat;
    mon_delta._z = getElevTot();
    mon_delta._t = getTempsTot();

    return mon_delta;
}

float ListePoints::getTempsTot() {
  return _lpoints.back()._rtime - _lpoints.front()._rtime;
}

float ListePoints::getElevTot() {
  return _lpoints.back()._alt - _lpoints.front()._alt;
}

