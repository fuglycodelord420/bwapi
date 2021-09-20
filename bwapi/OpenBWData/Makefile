SRCDIR := .
OPENBW_UI_DEFINE := -DOPENBW_ENABLE_UI
NAME := libOpenBWData
SHARED := true
# OPENBW_DIR

override CXXFLAGS += -fPIC

override CXXFLAGS += -I../BWAPI/Source -I../Util/Source -I../include -I../BWAPICore
override CXXFLAGS += $(OPENBW_UI_DEFINE)
# TODO: only if ui enabled
override LDLIBS += -lsimple_graphical -lsimple_interactive -lsimple_sdlcore -lSDL2 -lSDL2_mixer

ifneq ($(origin OPENBW_DIR), undefined)
override CXXFLAGS += -I$(OPENBW_DIR)/libopenbw_core/source -I$(OPENBW_DIR)/libopenbw_ui/source
override LOCALIB += $(OPENBW_DIR)/libopenbw_ui/out/libopenbw_ui.a
endif

LOCAL_MAKE_INCLUDE := include
override TEMPLATE := make_templates/static_lib
override LOCAL_TEMPLATE := $(LOCAL_MAKE_INCLUDE)/$(TEMPLATE)

ifneq ($(shell cat $(LOCAL_TEMPLATE) 2> /dev/null),)
include $(LOCAL_TEMPLATE)
else
include $(TEMPLATE)
endif
