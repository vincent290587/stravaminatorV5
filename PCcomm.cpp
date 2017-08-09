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

    yield();

  } // fin du fichier

  if (file.isOpen()) {
    file.close();
  }

  delay(100);

  uint8_t ind_lim = 0;
  while (download_request && ++ind_lim < 5) {
	  Serial.println(F("##LOG_STOP##"));
	  Serial.flush();
	  delay(1000);
  }

  Serial1.begin(9600);
  Serial3.begin(115200);

  return 0;
}


