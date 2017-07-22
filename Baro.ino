
void updateAltitude(float *_alt) {

    baro.getTempAndPressure(&att.temp, &att.pressu);
    /* Then convert the atmospheric pressure, and SLP to altitude*/
    *_alt = baro.pressureToAltitude(att.pressu);

}

void updateAltitudeOffset (float *_alt) {
  
  // pressure est mis a jour ici
  updateAltitude(_alt);
  // mise a jour de la pression au niveau de la mer
  baro.seaLevelForAltitude(att.gpsalt, att.pressu);
  // recalcul de falt avec la premiere mise a jour
  updateAltitude(_alt);
  // on set la correction
  baro.setCorrection(att.alt - att.gpsalt);
  
}



