ifeq (sam,$(BUILD_SPEC))

include mk/$(SAM_VARIANT).mk

MAP_FILE := $(OBJ_DIR)/$(TARGET).map
TARGET := $(TARGET).elf

include mk/arm_gcc.mk

# Target CPU architecture: cortex-m3, cortex-m4
ARCH ?= cortex-m7

# Application optimization used during compilation and linking:
# -O0, -O1, -O2, -O3 or -Os
OPTIMIZATION ?= -O0

FLAGS += -mcpu=$(ARCH) -mthumb

# set architecture to find freertos port-specific files
FREERTOS_PORT_DIR := ARM_CM7/r0p1

# ASF has lots of #include statements without any path component, requiring
# a huge set of include directories.
INCLUDE_DIRS += \
       -Isam/asf/                                        \
       -Isam/asf/common/                                 \
       -Isam/asf/common/boards/                          \
       -Isam/asf/sam/boards/                             \
       -Isam/asf/sam/utils/                              \
       -Isam/asf/common/utils/                           \
       -Isam/asf/sam/utils/preprocessor/                 \
       -Isam/asf/sam/utils/header_files/                 \
       -Isam/asf/sam/utils/cmsis/$(SAM_VARIANT)/include/          \
       -Isam/asf/sam/utils/cmsis/$(SAM_VARIANT)/source/templates/ \
       -Isam/asf/thirdparty/CMSIS/Include/               \
       -Isam/asf/sam/utils/fpu/                          \
       -Isam/asf/common/services/clock/                  \
       -Isam/asf/common/services/serial/                 \
       -Isam/asf/sam/drivers/efc/                        \
       -Isam/asf/sam/drivers/mcan/                       \
       -Isam/asf/sam/drivers/mpu/                        \
       -Isam/asf/sam/drivers/pio/                        \
       -Isam/asf/sam/drivers/pmc/                        \
       -Isam/asf/sam/drivers/rtt/                        \
       -Isam/asf/sam/drivers/spi/                        \
       -Isam/asf/sam/drivers/twihs/                      \
       -Isam/asf/sam/drivers/uart/                       \
       -Isam/asf/sam/drivers/usart/                      \
       -Isam/asf/common/services/ioport/                 \
       -Isam/asf/common/services/sleepmgr/               \
       -Isam/asf/common/services/usb/udc/                \

VPATH += sam/asf

# These source files are pretty much required for any project
SRC += \
       sam/utils/syscalls/gcc/syscalls.c                             \
       common/utils/interrupt/interrupt_sam_nvic.c                   \
       common/services/clock/$(SAM_VARIANT)/sysclk.c                  \
       common/services/delay/sam/cycle_counter.c                     \
       common/utils/stdio/read.c                                     \
       common/utils/stdio/write.c                                    \
       sam/drivers/pmc/pmc.c

# startup code is processor specific, but SAM_VARIANT should find it
SRC += sam/utils/cmsis/$(SAM_VARIANT)/source/templates/gcc/startup_$(SAM_VARIANT).c  \
       sam/utils/cmsis/$(SAM_VARIANT)/source/templates/system_$(SAM_VARIANT).c

# These source files are only needed if your project uses common peripherals.
# Should maybe be removed from here and moved to the individual project Makefiles.
SRC += \
       common/services/serial/usart_serial.c              \
       common/services/spi/sam_spi/spi_master.c           \
       sam/drivers/uart/uart.c                            \
       sam/drivers/usart/usart.c                          \
       sam/drivers/mcan/mcan.c                            \

# need to leave out -Wpedantic for dozens of "error: ISO C forbids conversion of function pointer to object pointer type"
obj/sam/sam/utils/cmsis/$(SAM_VARIANT)/source/templates/gcc/startup_$(SAM_VARIANT).o : FLAGS += -Wno-pedantic
# needed to avoid errors about unused zero-length CAN RX buffer arrays, and dead code that would access them
obj/sam/sam/drivers/mcan/mcan.o : FLAGS += -Wno-pedantic -Wno-array-bounds

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
#LIBS +=  -Lsam/asf/thirdparty/CMSIS/Lib/GCC/ -larm_cortexM7lfdp_math -larm_cortexM7l_math

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

OPENOCD_FLASH_COMMAND = \
    -c init -c targets -c "halt" \
    -c "flash write_image erase $(TARGET)" \
    -c "verify_image $(TARGET)" \
    -c "reset run" -c shutdown

openocd:
	openocd -f $(OPEN_OCD_CONFIG)

install: all
	@echo "Need to flash using JTAG"
	openocd -f $(OPEN_OCD_CONFIG) $(OPENOCD_FLASH_COMMAND)

debug.gdb: all
	$(GDB) $(TARGET)
	#inside gdb: target extended-remote localhost:3333
	#inside gdb: monitor halt reset
	#inside gdb: continue
	#inside gdb: ctrl-c
	# the below based on sam/asf/sam/utils/make/Makefile.sam.in
	# the below uses jlink, not openocd and edbg!
	#$(GDB) -x "$(DEBUG_SCRIPT)" -ex "reset" -readnow -se $(TARGET)

include mk/eclipse.mk
include mk/vscode.mk

#debug: debug.gdb
#debug: debug.eclipse
debug: debug.vscode

endif
