FREERTOS_INC_DIR=FreeRTOS/Source/include
PORTABLE_INC_DIR=FreeRTOS/Source/portable/GCC/${FREERTOS_PORT_DIR}

INCLUDE_DIRS += -I$(UCPLATFORM)/${FREERTOS_INC_DIR} -I$(UCPLATFORM)/${PORTABLE_INC_DIR}

FREERTOS_SRC := \
        ucplatform/FreeRTOS/Source/event_groups.c \
        ucplatform/FreeRTOS/Source/list.c         \
        ucplatform/FreeRTOS/Source/queue.c        \
        ucplatform/FreeRTOS/Source/tasks.c        \
        ucplatform/FreeRTOS/Source/timers.c       \
        ucplatform/FreeRTOS/Source/croutine.c     \
        ucplatform/FreeRTOS/Source/portable/GCC/${FREERTOS_PORT_DIR}/port.c \
        ucplatform/FreeRTOS/Source/portable/MemMang/heap_3.c

SRC += ${FREERTOS_SRC}
