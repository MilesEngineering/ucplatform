OPENOCD_FLASH_COMMAND = \
    -c init -c targets -c "halt" \
    -c "flash write_image erase $(TARGET)" \
    -c "verify_image $(TARGET)" \
    -c "reset run" -c shutdown

# run open ocd so debugger can connect to it
openocd:
	openocd -f $(OPEN_OCD_CONFIG)

# use openocd to flash the device
install.openocd:
	openocd -f $(OPEN_OCD_CONFIG) $(OPENOCD_FLASH_COMMAND)

# this launches command-line GDB, but should probably also do more commands to attach to OpenOCD.
debug.gdb: all
	@echo "type inside gdb> target extended-remote localhost:3333"
	@echo "type inside gdb> monitor halt reset"
	@echo "type inside gdb> continue"
	@echo "type inside gdb> ctrl-c"
	$(GDB) $(TARGET)
