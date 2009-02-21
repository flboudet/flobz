bin_PROGRAMS=fpserver
fpserver_SOURCES=PuyoServerIgpNatTraversal.cpp  PuyoServerMain.cpp \
                 PuyoServerIgpResponder.cpp PuyoPeersListener.cpp

fpserver_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/iosfc
fpserver_LDADD = -L../iosfc -liosfc

EXTRA_DIST=PuyoServerIgpNatTraversal.h  PuyoServerIgpResponder.h PuyoPeersListener.h
