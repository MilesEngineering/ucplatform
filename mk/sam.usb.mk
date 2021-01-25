# only include this in your Makefile if you want USB CDC support

INCLUDE_DIRS += \
    -I$(ASF)/common/services/usb/                                 \
    -I$(ASF)/common/services/usb/class/cdc/                       \
    -I$(ASF)/common/services/usb/class/cdc/device/                \

SRC += $(ASF)/common/services/usb/class/cdc/device/udi_cdc.c \
       $(ASF)/sam/drivers/usbhs/usbhs_device.c \
       $(ASF)/sam/drivers/pmc/sleep.c \
       $(ASF)/common/services/sleepmgr/sam/sleepmgr.c \
       $(ASF)/common/services/usb/udc/udc.c \
       $(ASF)/common/services/usb/class/cdc/device/udi_cdc_desc.c

# prevent warnings for zero-length array and use of possibly unintialized memory
$(OBJ_DIR)/$(ASF)/common/services/usb/udc/udc.o : FLAGS += -Wno-pedantic -Wno-maybe-uninitialized
