PROJECT_NAME     := stravaminator
TARGETS          := stravaminatorV5
OUTPUT_DIRECTORY := _build

CMSIS_PATH := ../CMSIS/Lib/GCC
SDK_ROOT := ../cores/teensy3
PROJ_DIR := .

$(OUTPUT_DIRECTORY)/stravaminatorV5.out: \
  LINKER_SCRIPT  := $(SDK_ROOT)/mk64fn512.ld

include $(PROJ_DIR)/Makefile.files

# Source files
SRC_FILES += \
  $(wildcard $(SDK_ROOT)/*.c) \
  $(wildcard $(SDK_ROOT)/*.cpp)\
  
# Include folders
INC_FOLDERS += \
  $(SDK_ROOT) \
  $(SDK_ROOT)/util \
  
# additional libraries to link
LIBS = -larm_cortexM4lf_math -lm -lstdc++
  
# CPPFLAGS = compiler options for C and C++
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
#CFLAGS += -Wall -Og -g
CFLAGS += -Wall -Ofast
CFLAGS += -DF_CPU=96000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -DUSING_MAKEFILE
CFLAGS += -D__MK64FX512__ -D__MK6X__ -DARDUINO=10613 -DTEENSYDUINO=132
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -MMD -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums -nostdlib -fno-exceptions

# C++ flags common to all targets
CXXFLAGS += -std=gnu++11 -felide-constructors -fno-exceptions -fno-rtti


# Assembler flags common to all targets
ASMFLAGS += -x assembler-with-cpp
#ASMFLAGS += -D__HEAP_SIZE=256


# Linker flags
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -L $(CMSIS_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections,--defsym=__rtc_localtime=0
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys


.PHONY: $(TARGETS) default all clean help flash flash_softdevice

# Default target - first one defined
default: stravaminatorV5

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	stravaminatorV5

TEMPLATE_PATH := $(PROJ_DIR)/../toolchain

include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

doc:
	cd $(PROJ_DIR)/ && doxygen ble_app_kien.doxyfile
	

# Flash the program
flash: $(OUTPUT_DIRECTORY)/stravaminatorV5.hex
	@echo Flashing: $<


# Flash softdevice
flash_softdevice:
	@echo Flashing: s332_nrf52_4.0.2_softdevice.hex


erase:
	@echo --eraseall
