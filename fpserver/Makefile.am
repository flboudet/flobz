if ENABLE_NETWORK_SERVER
bin_PROGRAMS=fpserver
fpserver_SOURCES=PuyoServerIgpNatTraversal.cpp  PuyoServerMain.cpp \
                 PuyoServerV1.cpp PuyoServerV2.cpp

fpserver_CXXFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/iosfc
fpserver_LDADD = -L../iosfc -liosfc -lsqlite3
endif
EXTRA_DIST=PuyoServerIgpNatTraversal.cpp PuyoServerMain.cpp  \
			PuyoServerV1.cpp PuyoServerV2.cppPuyoServerIgpNatTraversal.h \
			PuyoServerIgpResponder.h PuyoServer.h PuyoServerV1.h PuyoServerV2.h
