COMPONENTS = BWAPICore BWAPILIB BWAPI OpenBWData BWAPILauncher NullAIModule
export COPYRIGHT_FILE = ../../COPYRIGHT

ifneq ($(COMMON_BUILD_DIR),)
export PREFIX = $(COMMON_BUILD_DIR)
export override LDFLAGS += -Wl,-rpath,$(PREFIX)/lib
endif

all: $(COMPONENTS)
install: $(COMPONENTS)
uninstall: $(COMPONENTS)
clean: $(COMPONENTS)
distclean: $(COMPONENTS)

$(COMPONENTS):
	$(MAKE) -C $@ $(MAKECMDGOALS) --no-print-directory

.PHONY: all install uninstall clean distclean  $(COMPONENTS)
.NOTPARALLEL:
