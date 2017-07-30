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

//#define __DEBUG__
#define __DEBUG_STC__
//#define __DEBUG_GPS__
#define __DEBUG_GPS_S__
#define __DEBUG_NRF__
//#define __SST__
//#define __DEBUG_SST__


// structure SST26VF
typedef struct SInitTable {
  uint32_t secret_code;
  uint32_t last_data;
  uint32_t nb_data;
  uint32_t to_be_erased;
} SInitTable;

typedef union SWhiteBox {
  SInitTable sinit;
  uint8_t tab[12];
} SWhiteBox;


#endif

