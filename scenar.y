%{
#include <stdio.h>
#include "glSDL.h"
#include <string.h>
#include "PuyoCommander.h"
#include "IosImgProcess.h"
#include "PuyoStory.h"
#include "SDL_prim.h"
#include "sofont.h"

int yylex(void);
void yyerror(char *);
extern int linenum;

static IIM_Surface *first_layer = 0;
static IIM_Surface *back_layer  = 0;

float dec_vectorx;
float dec_vectory;

static float decx,decy;
static int txt_x,txt_y,txt_w,txt_h,txt_r;
static char txt[1024];

extern PuyoStory *theStory;

void loop_scenar(int time)
{
  Uint32 start = SDL_GetTicks();
  while (start + time * 1000 > SDL_GetTicks()) {
    SDL_Event e;

    while (SDL_PollEvent (&e)) {
      switch (e.type) {
        case SDL_QUIT:
          goto mml_fin;
        case SDL_KEYDOWN:
          if (e.key.keysym.sym == SDLK_RETURN) {
            goto mml_fin;
          }
          if (e.key.keysym.sym == SDLK_ESCAPE) {
            goto mml_fin;
          }
          break;
      }
    }

    decx += dec_vectorx;
    decy += dec_vectory;

    if (decx < 0) decx += back_layer->w;
    if (decy < 0) decy += back_layer->h;
    if (decx >= back_layer->w) decx -= back_layer->w;
    if (decy >= back_layer->h) decy -= back_layer->h;

    theCommander->updateAll(theStory);
  }
mml_fin:
  return;
}

void draw_scenar()
{
  SDL_Rect drect;
  // 1- Afficher le back layer
  if (back_layer) {
      drect.x = (short)decx - back_layer->w;
      drect.y = (short)decy;
      drect.w = back_layer->w;
      drect.h = back_layer->h;
      SDL_BlitSurface(back_layer->surf,NULL,display,&drect);
      drect.x = (short)decx;
      drect.y = (short)decy - back_layer->h;
      drect.w = back_layer->w;
      drect.h = back_layer->h;
      SDL_BlitSurface(back_layer->surf,NULL,display,&drect);
      drect.x = (short)decx - back_layer->w;
      drect.y = (short)decy - back_layer->h;
      drect.w = back_layer->w;
      drect.h = back_layer->h;
      SDL_BlitSurface(back_layer->surf,NULL,display,&drect);
      drect.x = (short)decx;
      drect.y = (short)decy;
      drect.w = back_layer->w;
      drect.h = back_layer->h;
      SDL_BlitSurface(back_layer->surf,NULL,display,&drect);
  }
  if (first_layer) {
      drect.x = 0;
      drect.y = display->h - first_layer->h;
      drect.w = first_layer->w;
      drect.h = first_layer->h;
      SDL_BlitSurface(first_layer->surf,NULL,display,&drect);
  }
  if (txt_w>0) {
      SDL_fillCircle(display, txt_x, txt_y+txt_r, txt_r, 0xffffffff);
      SDL_fillCircle(display, txt_x, txt_y+txt_h-txt_r, txt_r, 0xffffffff);
      SDL_fillCircle(display, txt_x+txt_w, txt_y+txt_r, txt_r, 0xffffffff);
      SDL_fillCircle(display, txt_x+txt_w, txt_y+txt_h-txt_r, txt_r, 0xffffffff);
      drect.x = txt_x;
      drect.y = txt_y;
      drect.w = txt_w;
      drect.h = txt_h+1;
      SDL_FillRect(display,&drect,0xffffffff);
      drect.x = txt_x-txt_r;
      drect.y = txt_y+txt_r;
      drect.w = txt_w+2*txt_r+1;
      drect.h = txt_h-2*txt_r;
      SDL_FillRect(display,&drect,0xffffffff);
      SoFont_PutString(theStory->commander->darkFont,display,txt_x,txt_y+10,txt,NULL);
  }
}

%}

%union {
    int i;
    char str[1024];
}

%token INTEGER STRING
%token BACKGROUND VECTOR
%token FOREGROUND
%token MUSIC
%token WAIT SEC LOOP
%token TEXTAREA NOTEXTAREA
%token COMMA
%token SAY

%type <str> STRING
%type <i>   INTEGER

%%

scenar: prologue instructions epilogue;

prologue: {
    first_layer = back_layer = 0;
    decx = decy = 0;
    dec_vectorx = 0;
    dec_vectory = 0;
    txt_w=txt_h=0;
    linenum=1;
};
epilogue: {
   if(first_layer) { IIM_Free(first_layer); first_layer=0; }
   if(back_layer)  { IIM_Free(back_layer);  back_layer=0;  }
};

instructions: instructions instruction
            |;

instruction:
        FOREGROUND STRING         {
          if(first_layer) { IIM_Free(first_layer); first_layer=0; }
          first_layer = IIM_Load_DisplayFormatAlpha($2);
        }
        |BACKGROUND STRING         {
          if(back_layer)  { IIM_Free(back_layer);  back_layer=0;  }
          back_layer = IIM_Load_DisplayFormatAlpha($2);
        }
        |VECTOR INTEGER COMMA INTEGER {
          dec_vectorx = 0.01f * $2;
          dec_vectory = 0.01f * $4;
        }
        |WAIT INTEGER SEC {
          loop_scenar($2);
        }
        |LOOP {
          loop_scenar(36000);
        }
        |TEXTAREA INTEGER COMMA INTEGER COMMA INTEGER COMMA INTEGER COMMA INTEGER {
          txt_x = $2;
          txt_y = $4;
          txt_w = $6;
          txt_h = $8;
          txt_r = $10;
        }
        |NOTEXTAREA {
          txt_w = txt_h = 0;
        }
        |SAY STRING {
          strcpy(txt,$2);
        }
        ;

%%

#include <stdlib.h>

void yyerror(char *s) {
    fprintf(stderr, "ERROR LINE %d: %s\n", linenum, s);
    exit(1);
}

extern int yyrestart(FILE*f);

void launch_scenar(const char *f) 
{
  FILE *yyin = fopen(f,"r");
  if (yyin==NULL) return;
  yyrestart(yyin);
  yyparse();
  fclose(yyin);
}

