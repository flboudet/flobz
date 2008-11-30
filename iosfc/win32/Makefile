SRCS=ios_win32datagramsocketimpl.cpp ios_win32socketimpl.cpp ios_win32socketaddressimpl.cpp ios_win32networkinterfacerequesterimpl.cpp

OBJECTS=${SRCS:%.cpp=%.o}

CXXFLAGS:=${CXXFLAGS} -I. -I..

-include ../Config.mk

all: prelude libiosfc_win32.a

libiosfc_win32.a: ${OBJECTS}
	${AR} -cru libiosfc_win32.a ${OBJECTS}

prelude:
	@echo "PLATFORM: $(PLATFORM)"
	@echo "PLATFORMTYPE: $(PLATFORMTYPE)"

.PHONY: all prelude

-include ../Rules.mk
# DO NOT DELETE
