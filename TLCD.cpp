
#include "TLCD.h"
#include "utils.h"
#include "Segment.h"
#include "Global.h"

using namespace mvc;


TLCD::TLCD(uint8_t ss) : TSharpMem(SPI_CLK, SPI_MOSI, SHARP_CS), IntelliScreen() {

	_seg_act = 0;
	_nb_lignes_tot = 7;

	boot.nb_seg = -1;

	_parc = NULL;
	_points = NULL;

}

void TLCD::registerSegment(Segment *seg) {

	if (_seg_act < NB_SEG_REG && seg->longueur() > 2) {
		_l_seg[_seg_act] = seg;
		_seg_act += 1;
	}
}

void TLCD::registerParcours(Parcours *par) {

	_parc = par;
	_par_act = 1;

}

void TLCD::registerHisto(ListePoints *pts) {

	_points = pts;

}

void TLCD::resetSegments() {
	_seg_act = 0;
}

void TLCD::resetParcours() {
	_par_act = 0;
}

void TLCD::updatePos(float lat_, float lon_, float alt_) {
	_lat = lat_;
	_lon = lon_;
	_alt = alt_;
}

void TLCD::cadranH(uint8_t p_lig, const char *champ, String  affi, const char *p_unite) {

	int decal = 0;
	int x = LCDWIDTH / 2 * 0.5;
	int y = LCDHEIGHT / NB_LIG * (p_lig - 1);

	setTextColor(CLR_NRM); // 'inverted' text
	setCursor(5, y + 5);
	setTextSize(1);

	if (champ) print(champ);

	if (affi.length() < 4) {
		decal = (4 - affi.length()) * 20;
	}
	setCursor(x + 20 + decal, y + 20);
	setTextSize(3);
	print(affi);

	setTextSize(1);
	x = LCDWIDTH / 2;
	setCursor(x + 105, y + 5);// y + 42

	if (p_unite) print(p_unite);

	// print delimiters
	if (p_lig > 1) drawFastHLine(0, LCDHEIGHT / NB_LIG * (p_lig - 1), LCDWIDTH, BLACK);
	if (p_lig < NB_LIG) drawFastHLine(0, LCDHEIGHT / NB_LIG * (p_lig), LCDWIDTH, BLACK);
}


void TLCD::cadran(uint8_t p_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite) {

	int decal = 0;
	int x = LCDWIDTH / 2 * (p_col - 1);
	int y = LCDHEIGHT / NB_LIG * (p_lig - 1);

	setTextColor(CLR_NRM); // 'inverted' text
	setCursor(x + 5, y + 5);
	setTextSize(1);

	if (champ) print(champ);

	if (affi.length() < 6) {
		decal = (4 - affi.length()) * 14;
	} else if (affi.length() > 5) {
		affi = "---";
	}
	setCursor(x + 25 + decal, y + 23);
	setTextSize(3);
	print(affi);

	setTextSize(1);
	setCursor(x + 95, y + 5); // y + 42

	if (p_unite) print(p_unite);

	// print delimiters
	drawFastVLine(LCDWIDTH / 2, LCDHEIGHT / NB_LIG * (p_lig - 1), LCDHEIGHT / NB_LIG, BLACK);

	if (p_lig > 1) drawFastHLine(LCDWIDTH * (p_col - 1) / 2, LCDHEIGHT / NB_LIG * (p_lig - 1), LCDWIDTH / 2, BLACK);
	if (p_lig < NB_LIG) drawFastHLine(LCDWIDTH * (p_col - 1) / 2, LCDHEIGHT / NB_LIG * p_lig, LCDWIDTH / 2, BLACK);
}


void TLCD::updatePos() {
	updatePos(att.lat, att.lon, att.alt);
}


void TLCD::afficheBoot() {

	setTextColor(CLR_NRM); // 'inverted' text
	setCursor(40, 133);
	setTextSize(3);

	println("Booting...");
	println("");
	setTextSize(2);
	if (boot.sd_ok == -1) println( String(" SD: --"));
	if (boot.sd_ok == 1)  println( String(" SD: ok"));
	println("");
	if (boot.nb_seg != -1) println(String(" Nb segments: ") + boot.nb_seg);

}

void TLCD::printBatt() {

	// SOC
	setTextSize(1);
	setCursor(70, 5);
	print(att.pbatt);

	// charge
	setTextSize(1);
	setCursor(70, 15);
	//TODO print(cbatt, cbatt*cbatt > 100 ? 0 : 1);

	// icone
	fillRect(120, 10, 3, 6, BLACK); // petit bout
	drawRect(90, 7, 30, 12, BLACK); // forme exterieure
	if (att.pbatt > 3) {
		int Blevel = regFenLim(att.pbatt, 0., 100., 1., 26.);
		fillRect(92, 9, Blevel, 8, BLACK);
	}

	// current
	setTextSize(2);
	setCursor(5, 5);
	print(att.cbatt, fabs(att.cbatt) > 10 ? 0 : 1);

	// Battery voltage
	setCursor(10, 20);
	setTextSize(1);
	print(att.vbatt);

}

void TLCD::afficheGPS() {

	int largeur_b = 0;
	int16_t x, y;

	this->printBatt();

	setTextColor(CLR_NRM); // 'inverted' text
	setCursor(60, 133);
	setTextSize(3);

	println("GPS...");
	println("");
	setTextSize(2);
	println( String(" Sat:  ") + boot.nb_satU + " / " + boot.nb_satIV);
	println("");
	println( String(" HDOP: ") + boot.hdop);
	println("");
	println("");
	print("  ");
	largeur_b = regFenLim(att.nbpts, 0, MIN_POINTS - 1, 0, 200 - 4);
	x = getCursorX();
	y = getCursorY();
	drawRect(x, y, 200, 10, BLACK);
	fillRect(x + 2, y + 2, largeur_b, 6, BLACK);
}

void TLCD::afficheHRM() {

	cadranH(1, "RR", String(att.rrint), "ms");

	cadranH(2, "Dur", String("0") + ":" + 0, 0);

	cadran(3, 1, "CAD", String(att.cad_rpm), "rpm");
	cadran(3, 2, "HRM", String(att.bpm), "bpm");

	cadran(7, 1, "Batt", String(att.pbatt), "%");

}

void TLCD::afficheHT() {

	float vmoy = 0.;
	unsigned long int hrs = 0, mns = 0;
	String mins = "00";

	if (att.nbsec_act > MIN_POINTS) {
		vmoy = att.dist / att.nbsec_act * 3.6;
		hrs = (float)att.nbsec_act / 3600.;
		mns = att.nbsec_act % 3600;
		mns = mns / 60;
		if (mns < 10) mins = "0";
		else mins = "";
		mins.append(String(mns));
	}

	cadranH(1, "Speed", String(att.cad_speed, 1), "km/h");
	cadranH(2, "Pwr", String(att.pwr), "W");

	cadran(3, 1, "CAD", String(att.cad_rpm), "rpm");
	cadran(3, 2, "HRM", String(att.bpm), "bpm");

	cadranH(4, "Dist", String(att.dist / 1000, 2), "km");

	cadran(6, 1, "Vmoy", String(vmoy, 2), "km/h");
	cadran(6, 2, "Dur", String(hrs) + ":" + mns, 0);

	cadran(7, 1, "Batt", String(att.pbatt), "%");
	cadran(7, 2, "Cur", String((int)att.cbatt), "mA");

}

void TLCD::afficheSegments(void) {
	float vmoy = 0., speed = 0.;
	unsigned long int hrs = 0, mns = 0;
	String mins = "00";

	if (att.gps_src == 1) {
		this->fillTriangle(230, 0, 240, 0, 240, 10, BLACK);
	}

	if (millis() - last_nrf_cad < CAD_SPEED_TIMEOUT_MS) {
		// speed from CAD
		speed =  att.cad_speed;
	} else {
		// speed from GPS
		speed = att.speed;
		if (_seg_act <= 1) {
			this->fillTriangle(110, LCDHEIGHT / NB_LIG, 120, LCDHEIGHT / NB_LIG, 120, LCDHEIGHT / NB_LIG + 10, BLACK);
		}
	}

	if (att.nbpts - MIN_POINTS > 0 && att.nbsec_act > MIN_POINTS) {
		vmoy = att.dist / att.nbsec_act * 3.6;
		hrs = (float)att.nbsec_act / 3600.;
		mns = att.nbsec_act % 3600;
		mns = mns / 60;
		if (mns < 10) mins = "0";
		else mins = "";
		mins.append(String(mns));
	}

	if (_seg_act == 0) {

		_nb_lignes_tot = 7;

		// ligne colonne
		cadran(1, 1, "Dist", String(att.dist / 1000., 1), "km");
		cadran(1, 2, "Pwr", String(att.pwr), "W");
		cadran(2, 1, "Speed", String(speed, 1), "km/h");
		cadran(2, 2, "Climb", String(att.climb, 0), "m");
		cadran(3, 1, "CAD", String(att.cad_rpm), "rpm");
		cadran(3, 2, "HRM", String(att.bpm), "bpm");
		cadran(4, 1, "PR", String(att.nbpr), 0);
		cadran(4, 2, "VA", String(att.vit_asc * 3.600, 1), "km/h");
		cadran(6, 1, "Vmoy", String(vmoy, 2), "km/h");
		cadran(6, 2, "Dur", String(hrs) + ":" + mins, 0);
		cadran(7, 1, "Batt", String(att.pbatt), "%");

		cadranH(5, "Next seg", String(att.next), "m");

		hrs = att.secj / 3600.;
		// secondes dans l'heure
		mns = att.secj  - hrs * 3600.;
		mns = mns / 60.;
		if (mns < 10) mins = "0";
		else mins = "";
		mins.append(String(mns));

		cadran(7, 2, "Time", String(hrs) + ":" + mins, 0);

	} else if (_seg_act == 1) {

		_nb_lignes_tot = 7;

		cadran(1, 1, "Dist", String(att.dist / 1000., 1), "km");
		cadran(1, 2, "Pwr", String(att.pwr), "W");
		cadran(2, 1, "Speed", String(speed, 1), "km/h");
		cadran(2, 2, "Climb", String(att.climb, 0), "m");
		cadran(3, 1, "CAD", String(att.cad_rpm), "rpm");
		cadran(3, 2, "HRM", String(att.bpm), "bpm");
		cadran(4, 1, "PR", String(att.nbpr), 0);
		cadran(4, 2, "VA", String(att.vit_asc * 3.600, 1), "km/h");

		if (_l_seg[0]->getStatus() != SEG_OFF) {
			partner(_l_seg[0]->getAvance(), _l_seg[0]->getCur(), NB_LIG);
		} else {
			partner(0, 100., NB_LIG);
		}
		afficheListePoints(NB_LIG - 2, 0, 0);

	} else if (_seg_act == 2) {

		_nb_lignes_tot = 8;
		// ligne colonne
		cadran(1, 1, "Speed", String(speed, 1), "km/h");
		cadran(1, 2, "Pwr", String(att.pwr), "W");
		cadran(2, 1, "VA", String(att.vit_asc * 3.600, 1), "km/h");
		cadran(2, 2, "HRM", String(att.bpm), "bpm");


		partner(_l_seg[0]->getAvance(), _l_seg[0]->getCur(), NB_LIG - 3);
		afficheListePoints(NB_LIG - 5, 0, 0);

		partner(_l_seg[1]->getAvance(), _l_seg[1]->getCur(), NB_LIG);
		afficheListePoints(NB_LIG - 2, 1, 0);

	}
}

void TLCD::afficheParcours(void) {
	float vmoy = 0.;

	String mins = "00";

	if (att.nbpts - MIN_POINTS > 0 && att.nbsec_act > MIN_POINTS) {
		unsigned long int mns = 0;

		vmoy = att.dist / att.nbsec_act * 3.6;

		mns = att.nbsec_act % 3600;
		mns = mns / 60;

		if (mns < 10) mins = "0";
		else mins = "";

		mins.append(String(mns));
	}

	if (_seg_act == 0) {

		_nb_lignes_tot = 7;

		// ligne colonne
		cadran(1, 1, "Dist", String(att.dist / 1000., 1), "km");
		cadran(1, 2, "Pwr", String(att.pwr), "W");
		cadran(2, 1, "Speed", String(att.speed, 1), "km/h");
		cadran(2, 2, "Climb", String(att.climb, 0), "m");
		cadran(3, 1, "CAD", String(att.cad_rpm), "rpm");
		cadran(3, 2, "HRM", String(att.bpm), "bpm");
		cadran(4, 1, "Batt", String(att.pbatt), "%");
		cadran(4, 2, "Vmoy", String(vmoy, 2), "km/h");
		//cadran(4, 2, "Dur", String(hrs) + ":" + mins, 0);

		afficheListeParcours(NB_LIG - 2);

	} else {

		_nb_lignes_tot = 7;

		// ligne colonne
		cadran(1, 1, "Dist", String(att.dist / 1000., 1), "km");
		cadran(1, 2, "Pwr", String(att.pwr), "W");
		cadran(2, 1, "Speed", String(att.speed, 1), "km/h");
		cadran(2, 2, "Climb", String(att.climb, 0), "m");
		cadran(3, 1, "CAD", String(att.cad_rpm), "rpm");
		cadran(3, 2, "HRM", String(att.bpm), "bpm");

		afficheListeParcours(NB_LIG - 2);

		partner(_l_seg[0]->getAvance(), _l_seg[0]->getCur(), 4);

	}
}


/**
 * Indicateur du pourcentage de perf en forme d'avion
 */
void TLCD::partner(float rtime, float curtime, uint8_t ligne) {

	int hl, ol, dixP;
	float indice;
	static int centre = LCDWIDTH / 2;

	if (curtime < 5.) {
		indice = rtime / 5.;
	} else {
		indice = rtime / curtime;
	}

	hl = LCDHEIGHT / NB_LIG * (ligne - 1) + 25;
	ol = 30;

	//fillRect(centre - 8, hl - 8, 18, 18, WHITE);
	fillRect(ol, hl, LCDWIDTH - 2 * ol, 3, BLACK);

	// on fait apparaitre l'indice de performance en %
	// -> si -0.25 centre en ol
	// -> si  0.25 centre en 240-ol
	//centre = ol + (LCDWIDTH - ol - ol) * (0.25 + indice) / (0.5);
	centre = regFenLim(indice, -0.25, 0.25, ol, LCDWIDTH - ol);
	if (centre < ol) {
		centre = ol;
	}
	else if (centre > LCDWIDTH - ol) {
		centre = LCDWIDTH - ol;
	}

	dixP = (LCDWIDTH - 2.*ol) * 10. / 50.;
	drawFastVLine(LCDWIDTH / 2 - dixP, hl + 2, 7, BLACK);
	drawFastVLine(LCDWIDTH / 2 + dixP, hl + 2, 7, BLACK);

	fillTriangle(centre - 7, hl + 7, centre, hl - 7, centre + 7, hl + 7, BLACK);
	setCursor(centre - 15, hl + 12);
	setTextSize(NB_LIG > 7 ? 1 : 2);
	setTextColor(CLR_NRM); // 'inverted' text
	print(String((int)(indice * 100.)));


	// marques
	drawFastVLine(LCDWIDTH / 2, hl - 12, 12, BLACK);

}


/**
 *
 */
void TLCD::afficheListePoints(uint8_t ligne, uint8_t ind_seg, uint8_t mode) {

	int posTrouve = 0;
	float minLat = 100.;
	float minLon = 400.;
	float minAlt = 10000.;
	float maxLat = -100.;
	float maxLon = -400.;
	float maxAlt = -100.;
	float curDist = 0.;
	float dDist = 0.;
	float maxDist = 0.;
	float maDist = 10000.;
	Point *pCourant, *pSuivant;
	Point *maPos;
	float maDpex = 0;
	float maDpey = 0;
	std::list<Point>::iterator _iter;
	ListePoints *liste;

	if (_l_seg[ind_seg]->longueur() < 4) return;

	uint16_t debut_cadran = LCDHEIGHT / NB_LIG * (ligne - 1);
	uint16_t fin_cadran   = LCDHEIGHT / NB_LIG * (ligne + 1);

	// print delimiters
	if (ligne > 1) drawFastHLine(0, debut_cadran, LCDWIDTH, BLACK);
	if (ligne < NB_LIG) drawFastHLine(0, fin_cadran, LCDWIDTH, BLACK);

	// on cherche la taille de fenetre
	liste = _l_seg[ind_seg]->getListePoints();
	maPos = liste->getFirstPoint();
	pCourant = maPos;
	pSuivant = maPos;
	uint16_t points_nb = 0;
	for (_iter = liste->getLPTS()->begin(); _iter != liste->getLPTS()->end();) {

		if (_l_seg[ind_seg]->getStatus() == SEG_OFF && points_nb++ > SEG_OFF_NB_POINTS) {
			break;
		}

		pCourant = _iter.operator->();
		_iter++;
		if (_iter == liste->getLPTS()->end()) break;
		pSuivant = _iter.operator->();

		if (!pSuivant->isValid() || !pCourant->isValid()) break;

		if (pCourant->_lat < minLat) minLat = pCourant->_lat;
		if (pCourant->_lon < minLon) minLon = pCourant->_lon;
		if (pCourant->_alt < minAlt) minAlt = pCourant->_alt;

		if (pCourant->_lat > maxLat) maxLat = pCourant->_lat;
		if (pCourant->_lon > maxLon) maxLon = pCourant->_lon;
		if (pCourant->_alt > maxAlt) maxAlt = pCourant->_alt;

		if (pSuivant->_lat < minLat) minLat = pSuivant->_lat;
		if (pSuivant->_lon < minLon) minLon = pSuivant->_lon;
		if (pSuivant->_alt < minAlt) minAlt = pSuivant->_alt;

		if (pSuivant->_lat > maxLat) maxLat = pSuivant->_lat;
		if (pSuivant->_lon > maxLon) maxLon = pSuivant->_lon;
		if (pSuivant->_alt > maxAlt) maxAlt = pSuivant->_alt;

		if (maDist > distance_between(pCourant->_lat, pCourant->_lon, _lat, _lon) && _l_seg[ind_seg]->getStatus() > SEG_OFF) {
			maDist = distance_between(pCourant->_lat, pCourant->_lon, _lat, _lon) + 0.1;
			maPos = pCourant;
		}

		maxDist += distance_between(pCourant->_lat, pCourant->_lon, pSuivant->_lat, pSuivant->_lon);

	}

	if (_l_seg[ind_seg]->getStatus() < SEG_OFF) {
		// last point
		maPos = pCourant;
	} else if (_l_seg[ind_seg]->getStatus() == SEG_OFF) {
		// first point
		maPos = liste->getFirstPoint();
		maDist = distance_between(maPos->_lat, maPos->_lon, _lat, _lon) + 0.1;

		// notre position doit etre dans le rectangle
		if (_lat < minLat) minLat = _lat;
		if (_lon < minLon) minLon = _lon;
		if (_alt < minAlt) minAlt = _alt;

		if (_lat > maxLat) maxLat = _lat;
		if (_lon > maxLon) maxLon = _lon;
		if (_alt > maxAlt) maxAlt = _alt;
	}

	// on essaye de rendre l'image carree: ratio 1:1
	//if (_l_seg[ind_seg]->getStatus() != SEG_OFF) {
	while (maxLon - minLon < (maxLat - minLat)) {
		minLon -= 0.0008;
		maxLon += 0.0008;
	}

	while (maxLat - minLat < 0.5 * (maxLon - minLon)) {
		minLat -= 0.0014;
		maxLat += 0.0014;
	}
	//}

	// marge TODO
	minLat -= 0.0014;
	minLon -= 0.0008;
	minAlt -= 15.;
	maxLat += 0.0014;
	maxLon += 0.0008;
	maxAlt += 15.;
	maxDist += 50.;

	curDist = 0.;
	// on affiche
	points_nb = 0;
	for (_iter = liste->getLPTS()->begin(); _iter != liste->getLPTS()->end(); ) {

		if (_l_seg[ind_seg]->getStatus() == SEG_OFF && points_nb++ > SEG_OFF_NB_POINTS) {
			break;
		}

		pCourant = _iter.operator->();
		_iter++;
		if (_iter == liste->getLPTS()->end()) break;
		pSuivant = _iter.operator->();

		if (!pSuivant->isValid() || !pCourant->isValid()) break;

		dDist = distance_between(pCourant->_lat, pCourant->_lon, pSuivant->_lat, pSuivant->_lon);

		if (mode == 0) {
			// mode segment
			drawLine(regFenLim(pCourant->_lon, minLon, maxLon, 0, LCDWIDTH),
					regFenLim(pCourant->_lat, minLat, maxLat, fin_cadran, debut_cadran),
					regFenLim(pSuivant->_lon, minLon, maxLon, 0, LCDWIDTH),
					regFenLim(pSuivant->_lat, minLat, maxLat, fin_cadran, debut_cadran), BLACK);
		} else {
			// altitude
			drawLine(regFenLim(curDist, -50., maxDist, 0, LCDWIDTH),
					regFenLim(pCourant->_alt, minAlt, maxAlt, fin_cadran, debut_cadran),
					regFenLim(curDist + dDist, -50., maxDist, 0, LCDWIDTH),
					regFenLim(pSuivant->_alt, minAlt, maxAlt, fin_cadran, debut_cadran), BLACK);
		}

		curDist += dDist;
		if (pCourant == maPos) posTrouve = 1;
		if (posTrouve == 0) {
			maDist = curDist;
		}
	}

	// draw a circle at the end of the segment
	if (_l_seg[ind_seg]->getStatus() != SEG_OFF) {
		drawCircle(regFenLim(pSuivant->_lon, minLon, maxLon, 0, LCDWIDTH),
				regFenLim(pSuivant->_lat, minLat, maxLat, fin_cadran, debut_cadran), 5, BLACK);
	} else {
		// draw a circle at the start of the segment
		maPos = liste->getFirstPoint();
		drawCircle(regFenLim(maPos->_lon, minLon, maxLon, 0, LCDWIDTH),
				regFenLim(maPos->_lat, minLat, maxLat, fin_cadran, debut_cadran), 5, BLACK);
	}

	// limit position when segment is finished
	if (_l_seg[ind_seg]->getStatus() < SEG_OFF) {
		_lon = pCourant->_lon;
		_lat = pCourant->_lat;
		_alt = pCourant->_alt;
	}

	// ma position
	if (mode == 0) {
		// mode segment
		maDpex = regFenLim(_lon, minLon, maxLon, 0, LCDWIDTH);
		maDpey = regFenLim(_lat, minLat, maxLat, fin_cadran, debut_cadran);
	} else {
		// altitude
		maDpex = regFenLim(maDist, -50., maxDist, 0, LCDWIDTH);
		maDpey = regFenLim(_alt, minAlt, maxAlt, fin_cadran, debut_cadran);
	}
	fillCircle(maDpex, maDpey, 4, BLACK);

	// return before printing text
	if (_l_seg[ind_seg]->getStatus() == SEG_OFF) {
		return;
	}

	setTextColor(CLR_NRM);
	if (maDpey > fin_cadran - 30) {
		setCursor(maDpex > LCDWIDTH - 70 ? LCDWIDTH - 70 : maDpex, maDpey - 20);
	} else {
		setCursor(maDpex > LCDWIDTH - 70 ? LCDWIDTH - 70 : maDpex, maDpey + 15);
	}

	setTextSize(2);

	print(String(_l_seg[ind_seg]->getAvance(), 1));
	// completion
	setCursor(10, debut_cadran + 10);
	print((int)(maDist / curDist * 100.));
	print("%");
}


void TLCD::afficheListeParcours(uint8_t ligne) {

	float minLat = 100.;
	float minLon = 400.;
	float maxLat = -100.;
	float maxLon = -400.;
	float dDist = 0.;
	float maDist = 10000.;
	Point *pCourant, *pSuivant;
	float maDpex = 0;
	float maDpey = 0;
	std::list<Point>::iterator _iter;
	ListePoints *liste;
	uint8_t nb_histo = 0;

	if (_parc->longueur() < 5) return;


	uint16_t debut_cadran = LCDHEIGHT / NB_LIG * (ligne - 1);
	uint16_t fin_cadran   = LCDHEIGHT / NB_LIG * (ligne + 2);

	// print delimiters
	if (ligne > 1) drawFastHLine(0, debut_cadran, LCDWIDTH, BLACK);
	if (ligne < NB_LIG) drawFastHLine(0, fin_cadran, LCDWIDTH, BLACK);

	// init
	liste = _parc->getListePoints();

	// on cherche la taille de fenetre

	// distance au parcours
	maDist = liste->dist(_lat, _lon);
	minLon = maxLon = _lon;
	minLat = maxLat = _lat;

	if (maDist > 300) {
		dDist = maDist + 300;
	} else {
		dDist = 300;
	}

	while (distance_between(minLat, minLon, maxLat, maxLon) < dDist) {

		minLon -= 0.0008;
		maxLon += 0.0008;
		minLat -= 0.0014;
		maxLat += 0.0014;

	}


	// on affiche
	for (_iter = liste->getLPTS()->begin(); _iter != liste->getLPTS()->end();) {

		pCourant = _iter.operator->();
		_iter++;
		if (_iter == liste->getLPTS()->end()) break;
		pSuivant = _iter.operator->();

		if (!pSuivant->isValid() || !pCourant->isValid()) break;


		if ((minLon < pCourant->_lon && pCourant->_lon < maxLon &&
				minLat < pCourant->_lat && pCourant->_lat < maxLat) ||
				(minLon < pSuivant->_lon && pSuivant->_lon < maxLon &&
						minLat < pSuivant->_lat && pSuivant->_lat < maxLat)) {

			drawLine(regFenLim(pCourant->_lon, minLon, maxLon, 0, LCDWIDTH),
					regFenLim(pCourant->_lat, minLat, maxLat, fin_cadran, debut_cadran),
					regFenLim(pSuivant->_lon, minLon, maxLon, 0, LCDWIDTH),
					regFenLim(pSuivant->_lat, minLat, maxLat, fin_cadran, debut_cadran), BLACK);
		}

	}


	if (_points) {

		if (_points->longueur() > 2) {

			// anciennes positions
			for (_iter = _points->getLPTS()->begin(); _iter != _points->getLPTS()->end(); _iter++) {

				if (_iter == liste->getLPTS()->end() || nb_histo > 15) break;

				pCourant = _iter.operator->();

				if (!pCourant) break;

				if (!pCourant->isValid()) {
					break;
				}

				nb_histo++;

				if ((minLon < pCourant->_lon && pCourant->_lon < maxLon &&
						minLat < pCourant->_lat && pCourant->_lat < maxLat)) {

					maDpex = regFenLim(pCourant->_lon, minLon, maxLon, 0, LCDWIDTH);
					maDpey = regFenLim(pCourant->_lat, minLat, maxLat, fin_cadran, debut_cadran);
					fillCircle(maDpex, maDpey, 3, BLACK);

				}
			}
		}
	}

	// position courante
	maDpex = regFenLim(_lon, minLon, maxLon, 0, LCDWIDTH);
	maDpey = regFenLim(_lat, minLat, maxLat, fin_cadran, debut_cadran);
	fillCircle(maDpex, maDpey, 6, BLACK);


	if (_seg_act > 0 && _l_seg[0]) {
		if (_l_seg[0]->getListePoints()) {

			// avance segment
			setTextSize(3);
			setCursor(maDpex + 15, maDpey - 20);
			print(String(_l_seg[0]->getAvance(), 1));

			// fin segment
			Point *dpseg = _l_seg[0]->getListePoints()->getLastPoint();
			maDpex = regFenLim(dpseg->_lon, minLon, maxLon, 0, LCDWIDTH);
			maDpey = regFenLim(dpseg->_lat, minLat, maxLat, fin_cadran, debut_cadran);
			if ((dpseg->_lon > minLon && dpseg->_lon < maxLon) &&
					(dpseg->_lat > minLat && dpseg->_lat < maxLat)) {
				drawCircle(maDpex, maDpey, 8, BLACK);
			}

			// completion
			setTextSize(2);
			setCursor(10, debut_cadran + 10);
			if (_l_seg[0]->getTempsTot() - _l_seg[0]->getAvance() > 5.) {
				float segperc = _l_seg[0]->getCur() / (_l_seg[0]->getTempsTot() - _l_seg[0]->getAvance());
				if (0. < segperc && segperc < 100.) {
					print((int)(segperc * 100.));
					print("%");
				}
			}
		}
	}

	setTextColor(CLR_NRM); // 'inverted' text

	if (maDpey > fin_cadran - 30) {
		setCursor(maDpex > LCDWIDTH - 70 ? LCDWIDTH - 70 : maDpex, maDpey - 20);
	} else {
		setCursor(maDpex > LCDWIDTH - 70 ? LCDWIDTH - 70 : maDpex, maDpey + 15);
	}

	setTextSize(2);

}

void TLCD::affiANCS() {
	if (l_notif.size() > 0) {
		if (_ancs_mode > 0) {
			fillRect(0, 0, LCDWIDTH, LCDHEIGHT / NB_LIG, WHITE);
			setCursor(5, 5);
			setTextSize(2);
			setTextColor(CLR_NRM); // 'inverted' text
			if (l_notif.front().type != 0) {
				print(l_notif.front().title);
				println(":");
			}
			print(l_notif.front().msg);
		} else {
			l_notif.pop_front();
			if (l_notif.size() > 0) {
				_ancs_mode = ANCS_TIMER;
			}
		}
	}
	decrANCS();
	return;
}

SNotif::SNotif(uint8_t type_, const char *title_, const char *msg_) {

	type = type_;
	msg = msg_;

	if (type_ == 0) {
		title = "";
	} else {
		title = title_;
	}
}

void TLCD::notifyANCS(uint8_t type_, const char *title_, const char *msg_) {

	l_notif.push_back(SNotif(type_, title_, msg_));
	_ancs_mode = ANCS_TIMER;

	return;
}


void TLCD::affichageMenu () {

	setCursor(0, 50);
	setTextSize(3);
	setTextColor(CLR_NRM);

	uint8_t i;

	for (i = 0; i < getNbElemMenu(); i++) {
		// left hor. space
		println(String("  "));

		if (i == getSelectionMenu()) setTextColor(CLR_INV);

		// print item
		println(getMenuItem(i));

		if (i == getSelectionMenu()) setTextColor(CLR_NRM);
	}

	setTextColor(CLR_NRM);
}
