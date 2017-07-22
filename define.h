#ifndef _VAR_DEFINE_
#define _VAR_DEFINE_

#define CPU_RESTART_ADDR             ((uint32_t *)0xE000ED0C)
#define CPU_RESTART_VAL              (0x5FA0004)
#define CPU_RESTART                  (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

#define TAILLE_LIGNE 250

#define TABLE                        0x0000
#define SECRET_CODE                  0xDEADBEEF
#define SST_RECORDING_START          0x10000
#define NB_SECTORS_REC               150

#define SPI_FLASH_32K_SIZE           32768UL //32K block
#define SPI_FLASH_64K_SIZE           65536UL //64K block
#define SPI_FLASH_PAGE_SIZE          256

#define __DEBUG__
//#define __DEBUG_STC__
//#define __DEBUG_GPS__
#define __DEBUG_NRF__
//#define __SST__
//#define __DEBUG_SST__

const uint8_t virtbtn0 = 2; // PTD0
const uint8_t virtbtn1 = 14; // PTD1
const uint8_t led = 17;
const uint8_t sd_cs = 22;// PTC1

const uint8_t sharp_cs = 23;

const uint8_t memCs = 15; //chip select      PTC0
const uint8_t memWp = 44; //write protection PTB23
const uint8_t memHold = 45; //hold           PTB22

static unsigned long age;

static unsigned long start;
static unsigned long time_c = 0;
static unsigned long lastFix = 0;

static uint8_t new_gps_data = 0;
static uint8_t new_hrm_data = 0;
static uint8_t new_cad_data = 0;
static uint8_t new_ancs_data = 0;
static uint8_t new_btn_data = 0;
static uint8_t new_dbg_data = 0;
static uint8_t download_request = 0;
static uint8_t upload_request = 0;
static uint8_t mode_simu = 0;
static uint8_t alertes_nb = 0;

// structure SST26VF
typedef struct SInitTable {
  uint32_t secret_code;
  uint32_t last_data;
  uint32_t nb_data;
} SInitTable;

typedef union SWhiteBox {
  SInitTable sinit;
  uint8_t tab[12];
} SWhiteBox;

// set up variables using the SD utility library functions:
SdFat sd;
SdFile file;
SdFile gpx;


SAttitude att;

ListeSegments mes_segments;

ListePoints   mes_points;

ListeParcours mes_parcours;

tBatteryData  batt_data;

#endif

