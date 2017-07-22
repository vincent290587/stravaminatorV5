
// initialise la liste des segments
// en listant les fichiers sur la SD
void initListeSegments() {

  static char chaine[20];
  Parcours *par_ = NULL;

  Serial.flush();
  Serial.println(F("Fichiers ajoutes:"));
  Serial.flush();

  if (file.isOpen())
    file.close();

  while (file.openNext(sd.vwd(), O_READ)) {

    if (file.isFile()) {
      // Indicate not a directory.
      file.getFilename(chaine);
      Serial.print(chaine);
      if (Segment::nomCorrect(chaine)) {
        Serial.println(F("  ajoute"));
        mes_segments.push_back(Segment(chaine));
      } else if (Parcours::nomCorrect(chaine)) {
        // pas de chargement en double
        Serial.println(F(": parcours trouve"));
        mes_parcours.push_back(Parcours(chaine));
        par_ = mes_parcours.getLastParcours();
        if (chargerPAR(par_) == 0) {
          display.registerParcours(par_);
          Serial.println(F("Parcours enregistre"));
        } else {
          Serial.println(F("Chargement parcours incorrect"));
        }

      } else {
        Serial.println(F(": probleme de nom de fichier"));
      }
    }


    if (file.isOpen())
      file.close();
  }

  return;
}


// charge physique le .crs depuis la SD
int chargerCRS(Segment *mon_segment) {

  int res = 0;
  static float time_start = 0.;
  char chaine[TAILLE_LIGNE];

  res = 0;
  time_start = 0.;

  if (file.isOpen()) {
    file.close();
  }
  if (mon_segment) {

    Serial.print(F("chargerCRS: ")); Serial.println(mon_segment->getName());
    Serial.print(F("Nb points: ")); Serial.println(mon_segment->longueur());

    Serial.flush();
    if (!file.open(mon_segment->getName(), O_READ)) {
      // echec d'ouverture
      Serial.print(F("cFichier introuvable:"));
      Serial.flush();
      Serial.println(mon_segment->getName());
      return 1;
    }


    while (file.fgets(chaine, TAILLE_LIGNE - 1, NULL)) {

      // on se met au bon endroit
      if (strstr(chaine, "<")) {
        // meta data
      } else if (strstr(chaine, ";")) {
        // on est pret a charger le point
        if (!chargerPointSeg(chaine, mon_segment, &time_start))
          res++;
      }

    } // fin du fichier
    file.close();
    Serial.println(F("Chargement effectue !"));
    Serial.print(F("Nb points: ")); Serial.println(mon_segment->longueur());
  } else {
    Serial.println(F("Segment vide"));
  }

  return res;
}

// charge physique le .crs depuis la SD
int chargerPAR(Parcours *mon_parcours) {

  int res = 0;
  char chaine[TAILLE_LIGNE];

  res = 0;

  if (file.isOpen()) {
    file.close();
  }
  if (mon_parcours) {

#ifdef __DEBUG__
    Serial.print(F("chargerPAR: ")); Serial.println(mon_parcours->getName());
    Serial.print(F("Nb points: ")); Serial.println(mon_parcours->longueur());
#endif

    Serial.flush();
    if (!file.open(mon_parcours->getName(), O_READ)) {
      // echec d'ouverture
      Serial.print(F("Fichier introuvable:"));
      Serial.flush();
      Serial.println(mon_parcours->getName());
      return 1;
    }


    while (file.fgets(chaine, TAILLE_LIGNE - 1, NULL)) {

      // on se met au bon endroit
      if (strstr(chaine, "<")) {
        // meta data
      } else if (strstr(chaine, ";")) {
        // on est pret a charger le point
        if (!chargerPointPar(chaine, mon_parcours))
          res++;
      }

    } // fin du fichier
    file.close();
#ifdef __DEBUG__
    Serial.println(F("Chargement effectue !"));
    Serial.print(F("Nb points: ")); Serial.println(mon_parcours->longueur());
#endif
  } else {
    Serial.println(F("Parcours vide"));
  }

  return 0;
}

// charge un unique point dans le fichier deja ouvert
int chargerPointSeg(char *buffer, Segment *mon_segment, float *time_start) {

  static int isError = 0;
  static float lon, lat, alt, rtime;
  static float data[4];
  uint8_t pos = 0;
  const char *deli = " ; ";
  char *pch;

  lat = 0; lon = 0; alt = 0;
  isError = 0;

  if (!buffer || !mon_segment || !time_start) return 1;

  // on se met au bon endroit
  pch = strtok (buffer, deli);
  while (pch != NULL && pos < 4)
  {
    data[pos] = strtof(pch, 0);
    pch = strtok (NULL, deli);
    pos++;
  }
  isError = pos != 4;

  if (!isError && mon_segment && time_start) {

    lat = data[0];
    lon = data[1];
    rtime = data[2];
    alt = data[3];

    if (mon_segment->longueur() > 0) {
      if (time_start) rtime -= *time_start;
      mon_segment->ajouterPointFin(lat, lon, alt, rtime);
    }
    else {
      // on init la liste
      if (time_start) *time_start = rtime;
      mon_segment->ajouterPointFin(lat, lon, alt, 0.);
    }

    return 0;

  }
  else {
    // echec
    Serial.println(F("Echec de lecture du point"));
    Serial.println(buffer);
    return isError;
  }
}

// charge un unique point dans le fichier deja ouvert
int chargerPointPar(char *buffer, Parcours *mon_parcours) {

  static int isError = 0;
  static float lon, lat;
  static float data[4];
  const char *deli = ";";
  uint8_t pos = 0;
  char *pch;

  lat = 0; lon = 0;
  isError = 0;

  if (!buffer || !mon_parcours) return 1;

  if (!strstr(buffer, deli)) return 1;

  // on se met au bon endroit
  pch = strtok (buffer, deli);
  while (pch != NULL && pos < 2)
  {
    data[pos] = strtof(pch, 0);
    pch = strtok (NULL, deli);
    pos++;
  }
  isError = pos != 2;

  if (!isError && mon_parcours) {

    lat = data[0];
    lon = data[1];

    mon_parcours->ajouterPointFin(lat, lon);

    return 0;

  }
  else {
    // echec
    Serial.println(F("Echec de lecture du point"));
    Serial.println(buffer);
    return isError;
  }
}
