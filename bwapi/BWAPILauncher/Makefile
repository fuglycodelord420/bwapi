SRCDIR := ./Source

override CPPFLAGS += -I../BWAPI/Source -I../Util/Source -I../OpenBWData -I../include -I../BWAPICore
override LDFLAGS	+= -L../OpenBWData/out -L../BWAPI/out -L../BWAPILIB/out
override LDLIBS += -lOpenBWData -lBWAPILIB -lBWAPI -lpthread

LOCAL_MAKE_INCLUDE := include
override TEMPLATE := make_templates/binary
override LOCAL_TEMPLATE := $(LOCAL_MAKE_INCLUDE)/$(TEMPLATE)

ifneq ($(shell cat $(LOCAL_TEMPLATE) 2> /dev/null),)
include $(LOCAL_TEMPLATE)
else
include $(TEMPLATE)
endif
