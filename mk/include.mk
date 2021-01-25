all:: echorule
clobber:: echorule
clean:: echorule

.PHONY : all clean clobber

echorule:
	@printf "\033[0;32m====> $(CURDIR) - $(BUILD_SPEC)\033[0m\n"

OBJ_DIR := obj/$(BUILD_SPEC)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

include mk/multibuild.mk
