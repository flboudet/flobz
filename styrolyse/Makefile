GSL_DIR=goomsl

CC=gcc
CFLAGS=-g -O0 -Wall -I${GSL_DIR} `sdl-config --cflags` # -DVERBOSE -DUSE_JITC_X86=1
# CFLAGS=-g -O3 -march=athlon -Wall -pedantic -DUSE_JITC_X86=1

GSL_OBJ=${GSL_DIR}/goomsl_yacc.o ${GSL_DIR}/goomsl_lex.o ${GSL_DIR}/goomsl_hash.o ${GSL_DIR}/goomsl_heap.o ${GSL_DIR}/goomsl.o ${GSL_DIR}/jitc_x86.o

OBJ=styrolyse.o

LIBS        =

all: gsl ${OBJ} main.o
	gcc -o styrolyse `sdl-config --libs` -lSDL_image ${GSL_OBJ} ${OBJ}

object: ${OBJ} gsl

gsl:
	make -C ${GSL_DIR} object

main.o: main.c

clean:
	rm -f ${GSL_OBJ} ${OBJ} *~

