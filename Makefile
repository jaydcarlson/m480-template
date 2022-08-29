######################################
# target
######################################
TARGET ?= "template"


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################

# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
src/main.c \
services/usb_vendor.c \
Device/Nuvoton/M480/Source/system_M480.c \
Device/Nuvoton/M480/Source/GCC/_syscalls.c \
StdDriver/src/acmp.c \
StdDriver/src/bpwm.c \
StdDriver/src/can.c \
StdDriver/src/ccap.c \
StdDriver/src/clk.c \
StdDriver/src/crc.c \
StdDriver/src/crypto.c \
StdDriver/src/dac.c \
StdDriver/src/eadc.c \
StdDriver/src/ebi.c \
StdDriver/src/ecap.c \
StdDriver/src/emac.c \
StdDriver/src/epwm.c \
StdDriver/src/fmc.c \
StdDriver/src/gpio.c \
StdDriver/src/hsusbd.c \
StdDriver/src/i2c.c \
StdDriver/src/i2s.c \
StdDriver/src/pdma.c \
StdDriver/src/qei.c \
StdDriver/src/qspi.c \
StdDriver/src/rtc.c \
StdDriver/src/sc.c \
StdDriver/src/scuart.c \
StdDriver/src/sdh.c \
StdDriver/src/spi.c \
StdDriver/src/spim.c \
StdDriver/src/sys.c \
StdDriver/src/timer_pwm.c \
StdDriver/src/timer.c \
StdDriver/src/trng.c \
StdDriver/src/uart.c \
StdDriver/src/usbd.c \
StdDriver/src/usci_i2c.c \
StdDriver/src/usci_spi.c \
StdDriver/src/usci_uart.c \
StdDriver/src/wdt.c \
StdDriver/src/wwdt.c \




# ASM sources
ASM_SOURCES =  \
Device/Nuvoton/M480/Source/GCC/startup_M480.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-ICMSIS/Include \
-IDevice/Nuvoton/M480/Include \
-IStdDriver/inc \
-Iinc \
-Iservices 


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = Device/Nuvoton/M480/Source/GCC/M480xxDAE.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	rm -rf $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***