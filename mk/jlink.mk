# Segger J-Link GUI debugger application is called Ozone
#
#
SEGGER_DOWNLOAD := https://www.segger.com/downloads/jlink/
JLINK_TOOLS_DEB := JLink_Linux_x86_64.deb
OZONE_DEB := Ozone_Linux_V322a_x86_64.deb
OZONE := /usr/bin/Ozone
JFLASH := /usr/bin/JFlash

.PHONY: debug.ozone

# If the Ozone project exists, open it.
# Otherwise we can create it by launching Ozone with the appropriate command-line arguments.
# Note that creating the project doesn't specify FreeRTOS-awareness!  If that becomes annoying,
# we can write a script to create the file (instead of invoking Ozone to do it).
# We don't want to check the file in because it has absolute paths in it.
OZONE_PROJECT := $(notdir $(basename $(TARGET))).jdebug
debug.ozone: | $(OZONE)
ifneq ("$(wildcard $(OZONE_PROJECT))","")
	$(OZONE) $(OZONE_PROJECT)
else
	@echo -e 'FreeRTOS awwareness, execute this in the Console of Ozone:\n    Project.SetOSPlugin("FreeRTOSPlugin_CM7")'
	$(OZONE) -programfile $(TARGET) -device $(JLINK_DEVICE) -select USB -if SWD -project $(OZONE_PROJECT)
endif

# This works with normal JLink, but not EDU model
install.jlink: | $(JFLASH)
	$(JFLASH) -hide -usb -open$(TARGET) -program -verify -exit


# if the Ozone binary doesn't exist at it's installed location, install it.
$(OZONE):
	# if the .deb package doesn't exist in current dir, wget it from Segger's server
	@if [ ! -f '$(OZONE_DEB)' ]; then \
	    echo "Downloading $(OZONE_DEB)" ; \
	    wget $(SEGGER_DOWNLOAD)/$(OZONE_DEB); \
	fi
	sudo apt-get install ./$(OZONE_DEB)

# if the JFlash binary doesn't exist at it's installed location, install JLink tools package
$(JFLASH):
	# if the .deb package doesn't exist in current dir, wget it from Segger's server
	@if [ ! -f '$(JLINK_TOOLS_DEB)' ]; then \
	    echo "Downloading $(JLINK_TOOLS_DEB)" ; \
	    wget $(SEGGER_DOWNLOAD)/$(JLINK_TOOLS_DEB); \
	fi
	sudo apt-get install ./$(JLINK_TOOLS_DEB)
