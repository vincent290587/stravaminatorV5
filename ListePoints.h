/* 
 * File:   ListePoints.h
 * Author: vincent
 *
 * Created on October 15, 2015, 3:30 PM
 */

#ifndef LISTEPOINTS_H
#define	LISTEPOINTS_H


#define NB_RECORDING 9

#include <list>
#include "utils.h"
#include "Vecteur.h"
#include "WProgram.h"

using namespace std;

class Vecteur;

class Point2D {
public:
    Point2D();
    Point2D(float lat, float lon);
    Point2D & operator=(const Point2D &point);
    Point2D & operator=(const Point2D *point);
    //void toString();
    //int isValid();
    float dist(Point2D *autre_p)  {
        return distance_between(_lat, _lon, autre_p->_lat, autre_p->_lon);
    }
public:
    float _lat;
    float _lon;
};

class Point : public Point2D {
public:
    Point();
    Point(float lat, float lon, float alt, float rtime);
    Point & operator=(const Point &point);
    Point & operator=(const Point *point);
    void toString();
    int isValid();
    float dist(Point *autre_p)  {
        return distance_between(_lat, _lon, autre_p->_lat, autre_p->_lon);
    }
    float dist(Point autre_p)  {
        return distance_between(_lat, _lon, autre_p._lat, autre_p._lon);
    }
    
public:
    float _alt;
    float _rtime;
};



class ListePoints {
public:
    ListePoints();
    void ajouteDebut(float lat, float lon, float alt, float msec);
    void ajouteFin(float lat, float lon, float alt, float msec);
    void enregistrePos(float lat, float lon, float alt, float msec);
    int  longueur();
    void vider();
    void supprLast();
    Point *getFirstPoint();
    Point *getLastPoint();
    Point *getPointAt(int i);
    void toString();
    Vecteur posRelative(Point point);
    Vecteur deltaListe();
    float dist(Point *p_);
	float dist(float lat_, float lon_);
    float getTempsTot();
    float getElevTot();
    std::list<Point> *getLPTS() {return &_lpoints;}
    
private:
    std::list<Point> _lpoints;
    
};

#endif	/* LISTEPOINTS_H */

