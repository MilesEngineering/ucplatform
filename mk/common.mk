# directory of this makefile, so we can include our siblings
MK_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
UCPLATFORM := $(shell basename $(dir $(abspath $(MK_DIR))))

include $(MK_DIR)/include.mk
include $(MK_DIR)/$(BUILD_SPEC).mk
include $(MK_DIR)/msg.mk
include $(MK_DIR)/freertos.mk

TARGET := $(OBJ_DIR)/$(TARGET)

all:: ${TARGET}

FLAGS := -Wall -Werror -Wpedantic -Wextra $(OPTIMIZATION) -g -MMD -DBUILD_SPEC_$(subst /,_,$(BUILD_SPEC))=1 $(FLAGS)

CFLAGS += ${FLAGS}
CXXFLAGS += ${FLAGS}
LDFLAGS += ${FLAGS}

CC := ${PREFIX}gcc
CXX := ${PREFIX}g++
GDB := ${PREFIX}gdb

INCLUDE_DIRS += -I$(UCPLATFORM) -I$(UCPLATFORM)/$(BUILD_SPEC)/include

OBJS := ${SRC:%.cpp=$(OBJ_DIR)/%.o}
OBJS := ${OBJS:%.c=$(OBJ_DIR)/%.o}
DEPS := ${OBJS:%.o=%.d}

# rules to compile and link
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	${CXX} ${INCLUDE_DIRS} ${CXXFLAGS} -c $< -o $@

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	${CC} ${INCLUDE_DIRS} ${CFLAGS} -c $< -o $@

$(TARGET): ${OBJS}
	${CXX} ${LDFLAGS} -o $@ ${OBJS} ${LIBS} 

# clean by deleting only files we intended to build
clean::
	-rm -f $(OBJS)
	-rm -f $(DEPS)
	-rm -f ${TARGET}

# clobber by removing whole directory
clobber::
	-rm -rf $(OBJ_DIR)

-include $(OBJS:%.o=%.d)
