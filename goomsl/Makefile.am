BUILT_SOURCES = goomsl_yacc.h
AM_YFLAGS = -d
AM_LFLAGS = -o$(LEX_OUTPUT_ROOT).c

noinst_LIBRARIES=libgoomsl.a

libgoomsl_a_SOURCES=goomsl_yacc.y goomsl_lex.l goomsl_hash.cpp goomsl_heap.c goomsl.c jitc_x86.c

libgoomsl_a_CXXFLAGS = -I../iosfc
