
#ifndef __FCT_XML__
#define __FCT_XML__

#include "Global.h"

// initialise la liste des segments
// en listant les fichiers sur la SD
void initListeSegments();


// charge physique le .crs depuis la SD
int chargerCRS(Segment *mon_segment);

// charge physique le .crs depuis la SD
int chargerPAR(Parcours *mon_parcours);

// charge un unique point dans le fichier deja ouvert
int chargerPointSeg(char *buffer, Segment *mon_segment, float *time_start);

// charge un unique point dans le fichier deja ouvert
int chargerPointPar(char *buffer, Parcours *mon_parcours);


int parseSegmentName(const char *name, float *lat_, float *lon_);

// desalloue automatiquement les segments lointains
// alloue automatiquement les segments proches

float watchdog(Segment *mon_seg, float lat1, float long1);


#endif