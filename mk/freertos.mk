FREERTOS_INC_DIR=FreeRTOS/Source/include
PORTABLE_INC_DIR=FreeRTOS/Source/portable/GCC/${FREERTOS_PORT_DIR}

INCLUDE_DIRS += -I$(UCPLATFORM)/${FREERTOS_INC_DIR} -I$(UCPLATFORM)/${PORTABLE_INC_DIR}

FREERTOS_SRC := \
        $(UCPLATFORM)/FreeRTOS/Source/event_groups.c \
        $(UCPLATFORM)/FreeRTOS/Source/list.c         \
        $(UCPLATFORM)/FreeRTOS/Source/queue.c        \
        $(UCPLATFORM)/FreeRTOS/Source/tasks.c        \
        $(UCPLATFORM)/FreeRTOS/Source/timers.c       \
        $(UCPLATFORM)/FreeRTOS/Source/croutine.c     \
        $(UCPLATFORM)/FreeRTOS/Source/portable/GCC/${FREERTOS_PORT_DIR}/port.c \
        $(UCPLATFORM)/FreeRTOS/Source/portable/MemMang/heap_3.c

SRC += ${FREERTOS_SRC}
