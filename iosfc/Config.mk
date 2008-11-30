# Platform Autodetection
ifndef PLATFORM
PLATFORM:=$(shell uname -s)
endif

# Default value for PlatformType
PLATFORMTYPE=POSIX

ifeq ($(PLATFORM), crossmingw32)
  PLATFORMTYPE=WIN32
endif

ifeq ($(PLATFORM), Linux)
    CXXFLAGS:=${CXXFLAGS} -DLINUX
endif

ifneq (, $(findstring CYGWIN, $(PLATFORM)))
  PLATFORMTYPE=WIN32
  CXXFLAGS:=${CXXFLAGS} -mno-cygwin
endif

ifeq ($(PLATFORMTYPE), WIN32)
  CXXFLAGS:=${CXXFLAGS} -DWIN32
endif

RANLIB?=ranlib
