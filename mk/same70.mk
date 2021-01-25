ifeq (same70,$(SAM_VARIANT))

FLAGS += -D BOARD=SAME70_XPLAINED -D __SAME70Q21B__

LINKER_SCRIPT = sam/asf/sam/utils/linker_scripts/same70/same70q21/gcc/flash.ld
DEBUG_SCRIPT = sam/asf/sam/boards/same70_xplained/debug_scripts/gcc/same70_xplained_flash.gdb

OPEN_OCD_CONFIG = /usr/share/openocd/scripts/board/atmel_same70_xplained.cfg

endif
