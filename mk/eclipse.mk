ECLIPSE_TARBALL := cdt-stand-alone-debugger-10.1.0-20201211-1436-linux.gtk.x86_64.tar.gz
ECLIPSE_DIR := eclipse

$(OBJ_DIR)/openocd.cdt:
	echo "org.eclipse.cdt.dsf.gdb/defaultGdbCommand=$(abspath $(GDB))" > $@

# need to launch openocd first!
debug.eclipse: | $(ECLIPSE_DIR) $(OBJ_DIR)/openocd.cdt
	$(ECLIPSE_DIR)/cdtdebug -pluginCustomization $(OBJ_DIR)/openocd.cdt -r 127.0.0.1:3333 -e $(TARGET)


$(ECLIPSE_DIR):
	@if [ ! -f '@(ECLIPSE_TARBALL)' ]; then \
	    wget https://mirror.umd.edu/eclipse/tools/cdt/releases/10.1/cdt-10.1.0/rcp/$(ECLIPSE_TARBALL); \
	fi
	mkdir $(ECLIPSE_DIR)
	tar -xvzf $(ECLIPSE_TARBALL) -C $(ECLIPSE_DIR) --strip-components 1
	rm $(ECLIPSE_TARBALL)
