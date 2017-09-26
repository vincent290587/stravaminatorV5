
#include <math.h>
#include "Parcours.h"

Parcours::Parcours(void) {
}

Parcours::Parcours(const char *nom_seg) {
	if (nom_seg)
		_nomFichier = nom_seg;
}

Parcours::Parcours(String nom_seg) {
	if (nom_seg.length() > 4)
		_nomFichier = nom_seg;
}

ListeParcours::ListeParcours(void) {

}

Point *Parcours::getFirstPoint() {
	return _lpts.getFirstPoint();
}

void Parcours::setParcoursName(const char *name_) {

	if (name_)
		_nomFichier = name_;

	return;
}

void Parcours::emptyName() {
	_nomFichier = "";
}


const char* Parcours::getName() {
	return _nomFichier.c_str();
}

int Parcours::isValid() {
	if (_nomFichier.length() > 0) {
		return 1;
	} else {
		return 0;
	}

}

void Parcours::desallouerPoints() {

	_lpts.vider();

	return;
}


void Parcours::ajouterPointFin(float lat, float lon) {

	_lpts.ajouteFin(lat, lon, 0., 0.);

	return;
}

void Parcours::ajouterPointDebutIso(float lat, float lon) {

	_lpts.ajouteDebut(lat, lon, 0., 0.);
	_lpts.supprLast();

}



void Parcours::toString() {

	//printf("Parcours:\nName: %s\nSize: %d\n", _nomFichier.c_str(), _lpts.longueur());
	_lpts.toString();

}

Vecteur Parcours::posAuParcours(Point point) {
	return _lpts.posRelative(point);
}


int Parcours::isNomCorrect() {

	if (_nomFichier.length() == 0) {
		return 0;
	}

	if (!_nomFichier.endsWith(".PAR")) {
		return 0;
	}

	return 1;
}



