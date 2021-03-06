ifeq (sam,$(BUILD_SPEC))

include $(MK_DIR)/$(SAM_VARIANT).mk

MAP_FILE := $(OBJ_DIR)/$(TARGET).map
TARGET := $(TARGET).elf

include $(MK_DIR)/arm_gcc.mk

# Target CPU architecture: cortex-m3, cortex-m4
ARCH ?= cortex-m7

# Application optimization used during compilation and linking:
# -O0, -O1, -O2, -O3 or -Os
OPTIMIZATION ?= -O0

FLAGS += -mcpu=$(ARCH) -mthumb

# set architecture to find freertos port-specific files
FREERTOS_PORT_DIR := ARM_CM7/r0p1

# full relative path to directory
ASF := $(UCPLATFORM)/sam/asf

# src dir should start with 'ucplatform', which must be added to VPATH
ASF_SRC := ucplatform/sam/asf

# ASF has lots of #include statements without any path component, requiring
# a huge set of include directories.
INCLUDE_DIRS += \
       -I$(ASF)/                                        \
       -I$(ASF)/common/                                 \
       -I$(ASF)/common/boards/                          \
       -I$(ASF)/sam/boards/                             \
       -I$(ASF)/sam/utils/                              \
       -I$(ASF)/common/utils/                           \
       -I$(ASF)/sam/utils/preprocessor/                 \
       -I$(ASF)/sam/utils/header_files/                 \
       -I$(ASF)/sam/utils/cmsis/$(SAM_VARIANT)/include/          \
       -I$(ASF)/sam/utils/cmsis/$(SAM_VARIANT)/source/templates/ \
       -I$(ASF)/thirdparty/CMSIS/Include/               \
       -I$(ASF)/sam/utils/fpu/                          \
       -I$(ASF)/common/services/clock/                  \
       -I$(ASF)/common/services/serial/                 \
       -I$(ASF)/sam/drivers/efc/                        \
       -I$(ASF)/sam/drivers/mcan/                       \
       -I$(ASF)/sam/drivers/mpu/                        \
       -I$(ASF)/sam/drivers/pio/                        \
       -I$(ASF)/sam/drivers/pmc/                        \
       -I$(ASF)/sam/drivers/rtt/                        \
       -I$(ASF)/sam/drivers/spi/                        \
       -I$(ASF)/sam/drivers/twihs/                      \
       -I$(ASF)/sam/drivers/uart/                       \
       -I$(ASF)/sam/drivers/usart/                      \
       -I$(ASF)/common/services/ioport/                 \
       -I$(ASF)/common/services/sleepmgr/               \
       -I$(ASF)/common/services/usb/udc/                \

# These source files are pretty much required for any project
SRC += \
       $(ASF_SRC)/sam/utils/syscalls/gcc/syscalls.c                             \
       $(ASF_SRC)/common/utils/interrupt/interrupt_sam_nvic.c                   \
       $(ASF_SRC)/common/services/clock/$(SAM_VARIANT)/sysclk.c                  \
       $(ASF_SRC)/common/services/delay/sam/cycle_counter.c                     \
       $(ASF_SRC)/common/utils/stdio/read.c                                     \
       $(ASF_SRC)/common/utils/stdio/write.c                                    \
       $(ASF_SRC)/sam/drivers/pmc/pmc.c

# startup code is processor specific, but SAM_VARIANT should find it
SRC += $(ASF_SRC)/sam/utils/cmsis/$(SAM_VARIANT)/source/templates/gcc/startup_$(SAM_VARIANT).c  \
       $(ASF_SRC)/sam/utils/cmsis/$(SAM_VARIANT)/source/templates/system_$(SAM_VARIANT).c

# These source files are only needed if your project uses common peripherals.
# Should maybe be removed from here and moved to the individual project Makefiles.
SRC += \
       $(ASF_SRC)/common/services/serial/usart_serial.c              \
       $(ASF_SRC)/common/services/spi/sam_spi/spi_master.c           \
       $(ASF_SRC)/sam/drivers/uart/uart.c                            \
       $(ASF_SRC)/sam/drivers/usart/usart.c                          \
       $(ASF_SRC)/sam/drivers/mcan/mcan.c                            \

# need to leave out -Wpedantic for dozens of "error: ISO C forbids conversion of function pointer to object pointer type"
$(OBJ_DIR)/$(ASF_SRC)/sam/utils/cmsis/$(SAM_VARIANT)/source/templates/gcc/startup_$(SAM_VARIANT).o : FLAGS += -Wno-pedantic
# needed to avoid errors about unused zero-length CAN RX buffer arrays, and dead code that would access them
$(OBJ_DIR)/$(ASF_SRC)/sam/drivers/mcan/mcan.o : FLAGS += -Wno-pedantic -Wno-array-bounds

ifneq ($(INCLUDE_SAM_USB_SUPPORT),)
include $(MK_DIR)/sam.usb.mk
endif

FLAGS += \
       -Wno-expansion-to-defined                          \
       -Wno-unused                                        \
       -D ARM_MATH_CM7=true                               \
       $(OPTIMIZATION)

# Note hard double-precision float (vs more common-mfloat-abi=softfp -mfpu=fpv5-sp-d16)
FLAGS += -mfloat-abi=hard -mfpu=fpv5-d16

# By default the linker gets the math library libm.a from toolchain at
# gcc-arm-none-eabi/arm-none-eabi/lib/thumb/v7e-m+dp/hard/libm.a
# Perhaps because this is C++ and libstdc++ depend on libm, it's
# seemingly impossible to make the linker use -larm_cortexM7lfdp_math -larm_cortexM7l_math
# to resolve math symbols instead of -lm from the toolchain.
# Because of that, adding -larm_cortexM7lfdp_math -larm_cortexM7l_math has
# no effect on the link map, and they might as well be left out entirely.
# It's unclear if the CMSIS version is better optimized, and we should make
# more effort to try to use it.
#LIBS +=  -L$(ASF)/thirdparty/CMSIS/Lib/GCC/ -larm_cortexM7lfdp_math -larm_cortexM7l_math

LDFLAGS += -Wl,-T $(LINKER_SCRIPT)

LDFLAGS += -pipe -Wl,--gc-sections

# needed for printf support of floats when using specs=nano.specs
# If we output debug messages as format string IDs and an array of
# parameters to do string conversion externally, this might not be
# needed.
LDFLAGS += -u _printf_float -u _scanf_float

LDFLAGS += -Wl,--entry=Reset_Handler -Wl,--cref -Wl,-Map=$(MAP_FILE),--cref
LDFLAGS += $(LIB_PATH)
LDFLAGS += -specs=nano.specs

# used to flash, and to debug with eclipse, vscode, qtcreator
#include $(MK_DIR)/openocd.mk

#include $(MK_DIR)/eclipse.mk
#include $(MK_DIR)/vscode.mk
include $(MK_DIR)/jlink.mk

# choose one of these for debugging
#debug: debug.gdb
#debug: debug.eclipse
#debug: debug.vscode
debug: debug.ozone

# choose one of these for flashing
#install: all install.openocd
install: all install.jlink

endif
