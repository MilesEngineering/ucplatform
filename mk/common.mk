# directory of this makefile, so we can include our siblings
MK_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# directory of UCPLATFORM, which is parent directory of this directory
UCPLATFORM := $(shell realpath --relative-to $(CURDIR) $(MK_DIR)/../)

# this is a little complicated, but the expected repo directory structure is:
# rootdir/
# rootdir/app1
# rootdir/app2
# rootdir/ucplatform
# That^ allows for multiple microcontroller projects to share one copy of
# ucplatform source code, but they each need to build it separately because
# they will potentially set up #defines differently, hence shouldn't share
# any object code.  In order for their obj directories to each have separate
# ucplatform, and NOT let the obj dir location for ucplatform files contain
# a .. in it, we need to add the parent directory of UCPLATFORM to VPATH.
UCPLATFORM_PARENT := $(shell realpath --relative-to $(CURDIR) $(UCPLATFORM)/../)
VPATH += $(UCPLATFORM_PARENT)

# if TARGET isn't set, default it to the dirname of the Makefile
ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
TARGET ?= $(notdir $(ROOT_DIR))

include $(MK_DIR)/include.mk
include $(MK_DIR)/$(BUILD_SPEC).mk
include $(MK_DIR)/msg.mk
include $(MK_DIR)/freertos.mk

TARGET := $(OBJ_DIR)/$(TARGET)
PRINTF_DICTIONARY_H := $(OBJ_DIR)/printf_dictionary.h
PRINTF_DICTIONARY_JSON := $(OBJ_DIR)/printf_dictionary.json

all:: ${TARGET}

FLAGS := -Wall -Werror -Wpedantic -Wextra $(OPTIMIZATION) -g -MMD -DBUILD_SPEC_$(subst /,_,$(BUILD_SPEC))=1 $(FLAGS)

CFLAGS += ${FLAGS}
CXXFLAGS += ${FLAGS}
LDFLAGS += ${FLAGS}

CC := ${PREFIX}gcc
CXX := ${PREFIX}g++
GDB := ${PREFIX}gdb

INCLUDE_DIRS += -I$(UCPLATFORM) -I$(UCPLATFORM)/$(BUILD_SPEC)/include

# for auto-generated $(PRINTF_DICTIONARY_H)
INCLUDE_DIRS += -I$(OBJ_DIR)

OBJS := ${SRC:%.cpp=$(OBJ_DIR)/%.o}
OBJS := ${OBJS:%.c=$(OBJ_DIR)/%.o}
DEPS := ${OBJS:%.o=%.d}

# rules to compile and link
$(OBJ_DIR)/%.o: %.cpp  $(PRINTF_DICTIONARY_H) | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	${CXX} ${INCLUDE_DIRS} ${CXXFLAGS} -c $< -o $@

$(OBJ_DIR)/%.o: %.c $(PRINTF_DICTIONARY_H) | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	${CC} ${INCLUDE_DIRS} ${CFLAGS} -c $< -o $@

$(TARGET): ${OBJS}
	${CXX} ${LDFLAGS} -o $@ ${OBJS} ${LIBS} 

$(PRINTF_DICTIONARY_H): $(SRC)
	msgfindprints .,$(UCPLATFORM) $(PRINTF_DICTIONARY_JSON) $(PRINTF_DICTIONARY_H)

# clean by deleting only files we intended to build
clean::
	-rm -f $(OBJS)
	-rm -f $(DEPS)
	-rm -f ${TARGET}

# clobber by removing whole directory
clobber::
	-rm -rf $(OBJ_DIR)

-include $(OBJS:%.o=%.d)
