
#ifdef __SST__
void resetWhiteBox() {

  uint8_t i;
  SWhiteBox whitebox;
  memset(&whitebox, 0, sizeof(whitebox));

  for (i = 0; i <= NB_SECTORS_REC; i++) {
    SerialFlash.eraseBlock(SST_RECORDING_START + i * SPI_FLASH_64K_SIZE);
  }

  whitebox.sinit.secret_code = SECRET_CODE;
  whitebox.sinit.last_data = SST_RECORDING_START;

  SerialFlash.eraseBlock(TABLE);
  SerialFlash.write(TABLE, whitebox.tab, sizeof(SInitTable));
  delay(5);
  Serial.println("Ecriture nouveau code " + sizeof(SInitTable));

}


uint32_t getLastAddress() {

  uint8_t i;
  SWhiteBox whitebox;
  memset(&whitebox, 0, sizeof(SInitTable));

  SerialFlash.read(TABLE, whitebox.tab, sizeof(SInitTable));

  if (whitebox.sinit.secret_code != SECRET_CODE) {
    Serial.print("Formattage SST26VF: ");
    Serial.println(whitebox.sinit.secret_code, HEX);

    resetWhiteBox();

    return SST_RECORDING_START;
  } else {
    return whitebox.sinit.last_data;
  }

}

void incrementeAddress () {

  SWhiteBox whitebox;
  memset(&whitebox, 0, sizeof(SWhiteBox));

  SerialFlash.read(TABLE, whitebox.tab, sizeof(SInitTable));

  if (whitebox.sinit.secret_code != SECRET_CODE) {
    Serial.print("Erreur code SST26VF: ");
    Serial.println(whitebox.sinit.secret_code, HEX);
    return;
  } else {
    whitebox.sinit.secret_code = SECRET_CODE;
    whitebox.sinit.last_data += sizeof(SAttitude);
    whitebox.sinit.nb_data += 1;
    // on efface le secteur de la whitebox
    SerialFlash.eraseBlock(TABLE);
    SerialFlash.write(TABLE, whitebox.tab, sizeof(SInitTable));
    Serial.print("Addresse courante: ");
    Serial.println(whitebox.sinit.last_data, HEX);
  }

}



void dumpLogGPS() {

  uint32_t i, adr_log;
  SWhiteBox whitebox;
  SBlackBox bbox;
  memset(&whitebox, 0, sizeof(SInitTable));
  String _line = "";

  Serial.println("Dump du log:");

  SerialFlash.read(TABLE, whitebox.tab, sizeof(SInitTable));

  if (whitebox.sinit.secret_code != SECRET_CODE) {
    Serial.println("Erreur check SST26VF: ");
    Serial.println(whitebox.sinit.secret_code, HEX);
    Serial.println(whitebox.sinit.last_data, HEX);
    Serial.println(whitebox.sinit.nb_data, HEX);
    return;
  } else {
    Serial.println("Check SST26VF OK");
    Serial.println(whitebox.sinit.secret_code, HEX);
    Serial.println(whitebox.sinit.last_data, HEX);
    Serial.println(whitebox.sinit.nb_data, DEC);
  }

  for (i = 0; i < whitebox.sinit.nb_data; i++) {

    adr_log = SST_RECORDING_START + i * sizeof(SAttitude);

    SerialFlash.read(adr_log, bbox.tab, sizeof(SAttitude));
    
#ifdef __DEBUG_SST__
    Serial.print("Point courant: ");
    Serial.print(bbox.satt.nbpts);
    Serial.print(" / lat: ");
    Serial.print(bbox.satt.lat);
    Serial.print(" / lon: ");
    Serial.println(bbox.satt.lon);
#else

    if (i==0) Serial.println(F("##LOG_START##"));

    _line = "";
    _line += String(bbox.satt.lat, 7) + ",";
    _line += String(bbox.satt.lon, 7) + ",";
    _line += String(bbox.satt.alt, 2) + ",";
    _line += String(bbox.satt.secj) + ",";
    _line += String(bbox.satt.bpm) + ",";
    _line += String(bbox.satt.cad_rpm) + ",";
    _line += String(bbox.satt.temp, 1) + ",";
    _line += String(bbox.satt.gpsalt, 2) + ",";
    _line += String(bbox.satt.vbatt, 3) + ",";
    _line += String(bbox.satt.cbatt, 3);

    Serial.println(_line);

    if (i==whitebox.sinit.nb_data-1) {
      Serial.println(F("##LOG_END##"));
      return;
    }
    
#endif
  }
}
#endif

void loggerMsg(const char *msg_) {

  Serial.println(msg_);

  if (gpx.isOpen()) gpx.close();
  if (gpx.open("errlog.txt", O_WRITE | O_CREAT | O_APPEND)) {
    gpx.println(msg_);
    // fclose
    gpx.close();
  }

}

void loggerMsg(int val_) {

  Serial.println(val_);

  if (gpx.isOpen()) gpx.close();
  if (gpx.open("errlog.txt", O_WRITE | O_CREAT | O_APPEND)) {
    gpx.println(val_);
    // fclose
    gpx.close();
  }

}

void loggerMsg(float val_1, float val_2) {

  String _line = "";

  if (gpx.isOpen()) gpx.close();
  if (gpx.open("errlog.txt", O_WRITE | O_CREAT | O_APPEND)) {

    _line += String(val_1, 7) + "@" + String(val_2, 7);
    gpx.println(_line);

    // fclose
    gpx.close();
  }

}


void loggerHT() {

  static int header_ecrit = 0;

  if (gpx.isOpen()) gpx.close();
  if (gpx.open("ht.csv", O_WRITE | O_CREAT | O_APPEND)) {

    // en tete
    if (header_ecrit == 0) {
      gpx.println(F("HRM,CAD,PW,VBatt"));
      header_ecrit = 1;
    }

    // corps
    gpx.print(att.bpm); gpx.print(",");

    gpx.print(att.cad_rpm); gpx.print(",");

    //TODO
    //gpx.print(calcul_puissance());gpx.print(",");

    // fclose
    gpx.close();

  }

}



void loggerRR() {

  static int header_ecrit = 0;

  if (gpx.isOpen()) gpx.close();
  if (gpx.open("rr.csv", O_WRITE | O_CREAT | O_APPEND)) {

    // en tete
    if (header_ecrit == 0) {
      gpx.println(F("HRM,RR,VBatt"));
      header_ecrit = 1;
    }

    // corps
    gpx.print(att.bpm); gpx.print(",");

    gpx.print(att.rrint); gpx.print(",");

    // fclose
    gpx.close();

  }

}

void loggerData() {

#ifdef __SST__
  uint32_t address_write;
  SBlackBox bbox;

  address_write = getLastAddress();

  memcpy(&bbox.satt, &att, sizeof(SAttitude));
  SerialFlash.write(address_write, bbox.tab, sizeof(SAttitude));

  // update current address
  incrementeAddress();
  Serial.println("Logging on the SST done");

  return;
#endif

  static int header_ecrit = 0;

  if (gpx.isOpen()) gpx.close();
  if (gpx.open("today.csv", O_WRITE | O_CREAT | O_APPEND)) {

    // en tete
    if (header_ecrit == 0) {
      ecrireHeader ();
      header_ecrit = 1;
    }

    // corps
    positionEcrit();

    // fclose
    gpx.close();

  }

}

void ecrireHeader () {

  gpx.print(F("Latitude,Longitude,Elevation,SecJour,HRM,"));
  gpx.println(F("CAD,Temp,GPSElevation,Vbatt,Cbatt,Loop"));

}


void positionEcrit () {

  String _line = "";

  _line += String(att.lat, 7) + ",";
  _line += String(att.lon, 7) + ",";
  _line += String(att.alt, 2) + ",";
  _line += String(att.secj) + ",";
  _line += String(att.bpm) + ",";
  _line += String(att.cad_rpm) + ",";
  _line += String(att.temp, 1) + ",";
  _line += String(att.gpsalt, 2) + ",";
  _line += String(att.vbatt, 3) + ",";
  _line += String(att.cbatt, 3) + ",";
  _line += String(time_c);

  gpx.println(_line);

}

