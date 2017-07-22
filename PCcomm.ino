
// charge physique le .crs depuis la SD
int PChisto() {

  char chaine[TAILLE_LIGNE];

  if (file) {
    file.close();
  }

  if (!SD.open("TODAY.CSV", O_READ)) {
    // echec d'ouverture
    return 1;
  }

  Serial.println(F("##LOG_START##"));

  Serial1.end();
  Serial3.end();

  while (file.read(chaine, TAILLE_LIGNE - 1)) {

    Serial.print(chaine);
    do {
      delay(1);// 4
    } while (64 != Serial.availableForWrite());
    //delay(25);

  } // fin du fichier

  delay(1000);
  Serial.println(F("##LOG_STOP##"));

  if (file) {
    file.close();
  }

  return 0;
}


