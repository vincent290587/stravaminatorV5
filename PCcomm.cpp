#include "PCcomm.h"
#include "Global.h"

using namespace mvc;

// charge physique le .crs depuis la SD
int PChisto() {

  char chaine[TAILLE_LIGNE];

  if (file.isOpen()) {
    file.close();
  }

  if (!file.open("TODAY.CSV", O_READ)) {
    // echec d'ouverture
    return 1;
  }

  Serial.println(F("##LOG_START##"));

  Serial1.end();
  Serial3.end();

  while (file.fgets(chaine, TAILLE_LIGNE - 1, NULL)) {

    Serial.print(chaine);
    do {
      delay(1);// 4
    } while (64 != Serial.availableForWrite());
    //delay(25);

  } // fin du fichier

  delay(1000);
  Serial.println(F("##LOG_STOP##"));

  if (file.isOpen()) {
    file.close();
  }

  return 0;
}


