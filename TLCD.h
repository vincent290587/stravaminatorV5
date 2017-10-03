
#ifndef _TLCD_
#define _TLCD_

#include "TSharpMem.h"
#include "IntelliScreen.h"
#include "Segment.h"
#include "Parcours.h"
#include "utils.h"
#include "WProgram.h"

#define SEG_OFF_NB_POINTS   5

#define SPI_CLK   13
#define SPI_MOSI  11
#define SHARP_CS  23

#define NB_LIG    _nb_lignes_tot

#define CLR_INV WHITE, BLACK
#define CLR_NRM BLACK, WHITE

#define LCDWIDTH       SHARPMEM_LCDHEIGHT
#define LCDHEIGHT      SHARPMEM_LCDWIDTH

#define BLACK 0
#define WHITE 1

#define NB_SEG_REG 2
#define ANCS_TIMER 5

typedef struct SAttitude { // definition d'un segment
  uint16_t nbpts;
  uint8_t nbact;
  uint8_t heure_c;
  uint8_t min_c;
  uint8_t has_started;
  float lat;
  float lon;
  float alt;
  float speed;
  float cad_speed;
  float gpsalt;
  float climb;
  float dist;
  uint16_t next;
  uint16_t nbsec_act;
  float secj;
  float secj_prec;
  uint8_t bpm;
  uint8_t cad_rpm;
  uint16_t pwr;
  uint8_t nbpr;
  uint8_t nbkom;
  float temp;
  float pressu;
  float vbatt;
  float cbatt;
  uint8_t pbatt;
  float rrint;
  float vit_asc;
  uint8_t gps_src;
  float power;
} SAttitude;

typedef union SBlackBox {
  SAttitude satt;
  uint8_t tab[100];
} SBlackBox;


typedef struct SBoot { // definition d'un segment
  int16_t nb_seg;
  uint16_t nb_satU;
  uint16_t nb_satIV;
  int16_t sd_ok;
  unsigned long hdop;
} SBoot;

class SNotif {
  public:
    SNotif(uint8_t type_, const char *title_, const char *msg_);

    uint8_t type;
    String title;
    String msg;
};


class Segment;

class TLCD : public TSharpMem, public IntelliScreen {
  public:
    TLCD(uint8_t ss = SHARP_CS);
    void registerSegment(Segment *seg);
    void registerParcours(Parcours *par);
    void registerHisto(ListePoints *pts);

    void emptyParcours();

    void resetSegments(void);
    void resetParcours(void);

    void notifyANCS(uint8_t type_, const char *title_, const char *msg_);

    void setSD(int16_t status_) {
      boot.sd_ok = status_;
    }
    void setNbSeg(int16_t nb_) {
      boot.nb_seg = nb_;
    }
    void setNbSatU(uint16_t nb_) {
      boot.nb_satU = nb_;
    }
    void setNbSatIV(uint16_t nb_) {
      boot.nb_satIV = nb_;
    }
    void setHDOP (unsigned long hdop_) {
      if (hdop_ > 255) {
        boot.hdop = 255;
      } else {
        boot.hdop = hdop_;
      }
    }

  protected:
    void cadran(uint8_t p_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite);
    void cadranH(uint8_t p_lig, const char *champ, String  affi, const char *p_unite);
    void updatePos(float lat_, float lon_, float alt_);
    void updatePos();
    void printBatt(void);
    void afficheSegments(void);
    void afficheParcours(void);
    void afficheHRM(void);
    void afficheListePoints(uint8_t ligne, uint8_t ind_seg, uint8_t mode);
    void afficheListeParcours(uint8_t ligne);
    void partner(float rtime, float curtime, uint8_t ind);

    void afficheBoot();
    void afficheGPS();
    void afficheHT();
    void affichageMenu ();
    void decrANCS() {
      if (_ancs_mode > 0)_ancs_mode -= 1;
      return;
    }
    void affiANCS();

  protected:
    uint8_t _ss, _seg_act, _par_act;

  private:
    SBoot boot;

    uint8_t _ancs_mode;
    std::list<SNotif> l_notif;

    uint8_t _nb_lignes_tot;

    float _lat, _lon, _alt;
    Segment *_l_seg[NB_SEG_REG];
    Parcours *_parc;
    ListePoints *_points;
};

#endif

