BUILT_SOURCES = goomsl_yacc.h
AM_YFLAGS = -d
AM_LFLAGS = -o$(LEX_OUTPUT_ROOT).c

noinst_LIBRARIES=libgoomsl.a

libgoomsl_a_SOURCES=goomsl_yacc.y goomsl_lex.l goomsl_hash.cpp goomsl_heap.c goomsl.c jitc_x86.c

libgoomsl_a_CXXFLAGS = -I$(top_srcdir)/iosfc

EXTRA_DIST=goomsl.h goomsl_hash.h goomsl_heap.h \
           goomsl_private.h goomsl_yacc.h jitc_x86.h

