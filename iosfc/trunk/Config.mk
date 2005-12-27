# Platform Autodetection
ifndef PLATFORM
PLATFORM=$(shell uname -s)
endif

# Default value for PlatformType
PLATFORMTYPE=POSIX

ifeq ($(PLATFORM), crossmingw32)
  PLATFORMTYPE=WIN32
endif

ifeq ($(PLATFORM), Cygwin)
  PLATFORMTYPE=WIN32
endif

