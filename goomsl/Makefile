CC=gcc
CFLAGS=-g -O0 -Wall -pedantic # -DVERBOSE -DUSE_JITC_X86=1
#CFLAGS=-g -O3 -march=athlon -Wall -pedantic -DUSE_JITC_X86=1
OBJ=goomsl_yacc.o goomsl_lex.o goomsl_hash.o goomsl_heap.o goomsl.o main.o jitc_x86.o

LIBS        = -L/usr/X11R6/lib -L/usr/lib -L/usr/local/lib -lX11 -lXext \
	-lXxf86dga -lXxf86vm -lXv -lm

goomsl: ${OBJ}
	make -C TinyPTC
	gcc ${LIBS} -g -o goomsl TinyPTC/*.o *.o

object: ${OBJ}

goomsl_lex.c:goomsl_lex.l goomsl_yacc.c
	lex -ogoomsl_lex.c goomsl_lex.l

goomsl_yacc.c:goomsl_yacc.y
	bison -d -b goomsl_yacc goomsl_yacc.y
	mv goomsl_yacc.tab.c goomsl_yacc.c
	mv goomsl_yacc.tab.h goomsl_yacc.h

goomsl_yacc.o:goomsl_yacc.c
goomsl_lex.o:goomsl_lex.c
goomsl_hash.o:goomsl_hash.c
goomsl_heap.o:goomsl_heap.c
goomsl.o:goomsl.c
jitc_x86.o:jitc_x86.c
main.o:main.c

clean:
	make -C TinyPTC clean
	rm -f *.o *~ goomsl_lex.c goomsl_yacc.c goomsl_yacc.h goomsl

jitc_test:jitc_test.c jitc_x86.c jitc_x86.h
	gcc -Wall -g -DDISPLAY_GENCODE=1 -DDISPLAY_GENCODE_HEXA=1 -o jitc_test jitc_test.c jitc_x86.c
