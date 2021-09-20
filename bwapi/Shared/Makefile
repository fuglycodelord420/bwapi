SRCDIR := .

override CXXFLAGS += -I../Util/Source -I../include -I../ -I../BWAPI/Source -I../OpenBWData -I../BWAPICore

NAME := libShared
override CXXFLAGS += -fPIC

LOCAL_MAKE_INCLUDE := include
override TEMPLATE := make_templates/static_lib
override LOCAL_TEMPLATE := $(LOCAL_MAKE_INCLUDE)/$(TEMPLATE)

ifneq ($(shell cat $(LOCAL_TEMPLATE) 2> /dev/null),)
include $(LOCAL_TEMPLATE)
else
include $(TEMPLATE)
endif
