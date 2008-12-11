bin_PROGRAMS=fpserver
fpserver_SOURCES=PuyoServerIgpNatTraversal.cpp  PuyoServerMain.cpp \
                 PuyoServerIgpResponder.cpp

fpserver_CXXFLAGS = -I$(top_srcdir)/iosfc
fpserver_LDADD = -L../iosfc -liosfc

EXTRA_DIST=PuyoServerIgpNatTraversal.h  PuyoServerIgpResponder.h
