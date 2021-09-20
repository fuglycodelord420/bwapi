SRCDIR := .

override CXXFLAGS += -I../Util/Source -I../include

NAME := libBWAPICore
override CXXFLAGS += -fPIC

LOCAL_MAKE_INCLUDE := include
override TEMPLATE := make_templates/static_lib
override LOCAL_TEMPLATE := $(LOCAL_MAKE_INCLUDE)/$(TEMPLATE)

ifneq ($(shell cat $(LOCAL_TEMPLATE) 2> /dev/null),)
include $(LOCAL_TEMPLATE)
else
include $(TEMPLATE)
endif
