/*
   File:   Parcours.h
   Author: vincent

   Created on March 28, 2016, 14:32 PM
*/

#ifndef PARCOURS_H
#define	PARCOURS_H

#include "ListePoints.h"
#include "Vecteur.h"
#include <WProgram.h>


#include <list>
#include <WString.h>

using namespace std;


class Parcours {
  public:
    Parcours(void);
    Parcours(const char *nom_seg);
    void setParcoursName(const char *name_);
    void emptyName();
    void desallouerPoints(void);
    int  longueur() {
      return _lpts.longueur();
    }
    int isValid();
    const char* getName();
    void ajouterPointFin(float lat, float lon);
    void ajouterPointDebutIso(float lat, float lon);
    void toString();
    Vecteur posAuParcours(Point point);
    ListePoints *getListePoints() {
      return &_lpts;
    }
    int nomCorrect();
    static int nomCorrect(const char *chaine) {
      Parcours tmp(chaine);
      return tmp.nomCorrect();
    }
    Point *getFirstPoint();
    float dist(Point *p) {
      return _lpts.dist(p);
    }

  private:
    String _nomFichier; // contient la pos du 1er point et le nom
    ListePoints _lpts;
};

class ListeParcours {
  public:
    ListeParcours(void);
    void push_back(Parcours prc) {
      _parcs.push_back(prc);
    };
    Parcours *getLastParcours(void) {
      return &_parcs.back();
    }
    int size() {
      return _parcs.size();
    }

    std::list<Parcours> _parcs;
  private:
};



#endif	/* PARCOURS_H */

