# This .mk file just handles downloading the toolchain and setting the PREFIX
# to invoke the binaries in it.  CFLAGS and include paths that are microcontroller
# specific are done in .mk files for those microcontrollers.
TOOLCHAIN_TARBALL := gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
TOOLCHAIN_DIR = gcc-arm-none-eabi

$(TOOLCHAIN_DIR):
	@if [ ! -f '@(TOOLCHAIN_TARBALL)' ]; then \
	    wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/$(TOOLCHAIN_TARBALL); \
	fi
	mkdir $(TOOLCHAIN_DIR)
	tar -xvjf $(TOOLCHAIN_TARBALL) -C $(TOOLCHAIN_DIR) --strip-components 1

# everything we build depends on OBJ_DIR, so a shortcut to make everything we build
# require the toolchain, is to have OBJ_DIR depend on the toolchain
$(OBJ_DIR): $(TOOLCHAIN_DIR)

# set prefix to invoke toolchain binaries
PREFIX:=$(TOOLCHAIN_DIR)/bin/arm-none-eabi-
