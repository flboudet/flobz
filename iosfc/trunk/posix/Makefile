SRCS=ios_unixdatagramsocketimpl.cpp ios_unixsocketaddressimpl.cpp ios_unixsocketimpl.cpp ios_unixthreadimpl.cpp ios_unixselectorimpl.cpp

OBJECTS=${SRCS:%.cpp=%.o}

CXXFLAGS:=${CXXFLAGS} -I. -I..

-include ../Config.mk

all: prelude libiosfc_posix.a

libiosfc_posix.a: ${OBJECTS}
	${AR} -cru libiosfc_posix.a ${OBJECTS}

prelude:
	@echo "PLATFORM: $(PLATFORM)"
	@echo "PLATFORMTYPE: $(PLATFORMTYPE)"

.PHONY: all prelude

-include ../Rules.mk
# DO NOT DELETE
