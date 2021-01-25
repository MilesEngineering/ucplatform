# only include this in your Makefile if you want USB CDC support

INCLUDE_DIRS += \
    -Isam/asf/common/services/usb/                                 \
    -Isam/asf/common/services/usb/class/cdc/                       \
    -Isam/asf/common/services/usb/class/cdc/device/                \

SRC += common/services/usb/class/cdc/device/udi_cdc.c \
       sam/drivers/usbhs/usbhs_device.c \
       sam/drivers/pmc/sleep.c \
       common/services/sleepmgr/sam/sleepmgr.c \
       common/services/usb/udc/udc.c \
       common/services/usb/class/cdc/device/udi_cdc_desc.c

# prevent warnings for zero-length array and use of possibly unintialized memory
obj/$(BUILD_SPEC)/common/services/usb/udc/udc.o : FLAGS += -Wno-pedantic -Wno-maybe-uninitialized
