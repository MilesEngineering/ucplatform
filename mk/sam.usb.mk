# only include this in your Makefile if you want USB CDC support

INCLUDE_DIRS += \
    -I$(ASF)/common/services/usb/                                 \
    -I$(ASF)/common/services/usb/class/cdc/                       \
    -I$(ASF)/common/services/usb/class/cdc/device/                \

SRC += $(ASF_SRC)/common/services/usb/class/cdc/device/udi_cdc.c \
       $(ASF_SRC)/sam/drivers/usbhs/usbhs_device.c \
       $(ASF_SRC)/sam/drivers/pmc/sleep.c \
       $(ASF_SRC)/common/services/sleepmgr/sam/sleepmgr.c \
       $(ASF_SRC)/common/services/usb/udc/udc.c \
       $(ASF_SRC)/common/services/usb/class/cdc/device/udi_cdc_desc.c

# prevent warnings for zero-length array and use of possibly unintialized memory
$(OBJ_DIR)/$(ASF_SRC)/common/services/usb/udc/udc.o : FLAGS += -Wno-pedantic -Wno-maybe-uninitialized
