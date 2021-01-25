ifeq (samv71,$(SAM_VARIANT))

FLAGS += -D BOARD=SAMV71_XPLAINED_ULTRA -D __SAMV71Q21B__

LINKER_SCRIPT = sam/asf/sam/utils/linker_scripts/samv71/samv71q21/gcc/flash.ld
DEBUG_SCRIPT = sam/asf/sam/boards/samv71_xplained_ultra/debug_scripts/gcc/samv71_xplained_ultra_flash.gdb

OPEN_OCD_CONFIG = /usr/share/openocd/scripts/board/atmel_samv71_xplained_ultra.cfg

endif
