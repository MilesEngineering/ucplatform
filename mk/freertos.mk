FREERTOS_INC_DIR=FreeRTOS/Source/include
PORTABLE_INC_DIR=FreeRTOS/Source/portable/GCC/${FREERTOS_PORT_DIR}

INCLUDE_DIRS += -I${FREERTOS_INC_DIR} -I${PORTABLE_INC_DIR}

FREERTOS_SRC := \
        FreeRTOS/Source/event_groups.c \
        FreeRTOS/Source/list.c         \
        FreeRTOS/Source/queue.c        \
        FreeRTOS/Source/tasks.c        \
        FreeRTOS/Source/timers.c       \
        FreeRTOS/Source/croutine.c     \
        FreeRTOS/Source/portable/GCC/${FREERTOS_PORT_DIR}/port.c \
        FreeRTOS/Source/portable/MemMang/heap_3.c

SRC += ${FREERTOS_SRC}
