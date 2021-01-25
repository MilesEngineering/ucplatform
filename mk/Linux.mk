ifeq (Linux,$(BUILD_SPEC))

# only want -pthread for linux!
LDFLAGS += -pthread
# set architecture to find freertos port-specific files
FREERTOS_PORT_DIR:=Linux

endif
