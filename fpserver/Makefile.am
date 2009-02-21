bin_PROGRAMS=fpserver
fpserver_SOURCES=PuyoServerIgpNatTraversal.cpp  PuyoServerMain.cpp \
                 PuyoServerV1.cpp PuyoServerV2.cpp

fpserver_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/iosfc
fpserver_LDADD = -L../iosfc -liosfc

EXTRA_DIST=PuyoServerIgpNatTraversal.h PuyoServerIgpResponder.h PuyoServer.h PuyoServerV1.h PuyoServerV2.h
