# buildspec names match top-level subdirectories, so we have to declare the
# rules for them to be .PHONY, so make will actually let us build them and
# not check timestamp on the subdirectory
.PHONY: $(ALL_BUILD_SPECS)

# we want to make rules to build not only "buildspec", but also "buildspec.clean" and "buildspec.clobber"
ALL_MULTIBUILD_TARGETS := \
    $(ALL_BUILD_SPECS) \
    $(addsuffix .clean,$(ALL_BUILD_SPECS))   \
    $(addsuffix .clobber,$(ALL_BUILD_SPECS)) \
    $(addsuffix .debug,$(ALL_BUILD_SPECS))   \
    $(addsuffix .install,$(ALL_BUILD_SPECS)) \

# multibuild targets are of the form buildspec.target, and are built as "make BUILD_SPEC=buildspec target"
$(ALL_MULTIBUILD_TARGETS):
	@$(MAKE) BUILD_SPEC=$(basename $@) --no-print-directory $(subst .,,$(suffix $@))

build_all :
	@echo "building $(ALL_BUILD_SPECS)";\
	for spec in _dummy_ $(ALL_BUILD_SPECS); do\
		if [ "$$spec" = "_dummy_" ]; then\
			continue ;\
		fi;\
		echo " ";\
		"$(MAKE)" --no-print-directory $(MFLAGS) BUILD_SPEC=$$spec DEBUG_MODE=$(DEBUG_MODE) TRACE=$(TRACE) || exit;\
	done

clean_all :
	@echo "cleaning $(ALL_BUILD_SPECS)";\
	for spec in _dummy_ $(ALL_BUILD_SPECS); do\
		if [ "$$spec" = "_dummy_" ]; then\
			continue ;\
		fi;\
		echo " ";\
		"$(MAKE)" --no-print-directory $(MFLAGS) BUILD_SPEC=$$spec DEBUG_MODE=$(DEBUG_MODE) TRACE=$(TRACE) clean || exit;\
	done

clobber_all :
	rm -rf obj
