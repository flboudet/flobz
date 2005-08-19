#include "goomsl.h"
#include "TinyPTC/tinyptc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void gsl_print(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  char result[1024];
  char *msg  = (char*)GSL_LOCAL_PTR(gsl,local,"msg");
  int   argi;
  float argf;
  char *args;
  int size = strlen(msg);
  int i,j=0;
  if (size>1023) size=1023;
  result[0]=0;
  
  for(i=0;i<size;++i) {
    if (msg[i]!='%')
      result[j++]=msg[i];
    else {
      ++i;
      switch(msg[i]) {
        case 's':
          args = (char*)GSL_LOCAL_PTR(gsl,local,"args");
          strcpy(&result[j], args);
          j+=strlen(args);
          break;
        case 'i':
        case 'd':
          argi = GSL_LOCAL_INT(gsl,local,"argi");
          sprintf(&result[j],"%d",argi);
          j=strlen(result);
          break;
        case 'f':
        case 'g':
          argf = GSL_LOCAL_FLOAT(gsl,local,"argf");
          sprintf(&result[j],"%f",argf);
          j=strlen(result);
          break;
      }
    }
  }
  result[j]=0;
  printf("%s", result);
}

void gsl_read_integer(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  char buffer[256];
  fgets(buffer,255,stdin);
  GSL_GLOBAL_INT(gsl,"read_integer") = strtol(buffer,NULL,0);
}

void gsl_read_line(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  int  ret;
  char buffer[256];
  fgets(buffer,255,stdin);
  ret = gsl_malloc(gsl, strlen(buffer)+1);
  buffer[strlen(buffer)-1] = 0;
  strcpy(gsl_get_ptr(gsl,ret), buffer);
  GSL_GLOBAL_INT(gsl,"read_line") = ret;
}

void gsl_random(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  int max        = GSL_LOCAL_INT(gsl,local,"max");
  GSL_GLOBAL_INT(gsl,"random") = (int) ((double)max*rand()/(RAND_MAX+1.0));
}

void gsl_srandom(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  srand(getpid());
}

void gsl_strcmp(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  char *str1 = GSL_LOCAL_PTR(gsl,local,"s1");
  char *str2 = GSL_LOCAL_PTR(gsl,local,"s2");
  GSL_GLOBAL_INT(gsl,"strcmp") = strcmp(str1,str2);
}

void gsl_ext_free(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  int ptr = GSL_LOCAL_INT(gsl,local,"pointer");
  gsl_free_ptr(gsl,ptr);
}

static int *pixel = NULL;
static int width, height;

void gsl_ptc_open(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  int  ret;
  char *title     = GSL_LOCAL_PTR(gsl,local,"title");
  /* HashValue *ret  = GOOM_HASH_get(global,"ptc_open"); */
  width           = GSL_LOCAL_INT(gsl,local,"width");
  height          = GSL_LOCAL_INT(gsl,local,"height");
  
  ret = ptc_open (title, width, height);
  /*  *(int*)ret->ptr = ptc_open (title, width, height); */
  if (ret)
    pixel = malloc(sizeof(int) * width * height);
  GSL_GLOBAL_INT(gsl, "ptc_open") = ret;
}

void gsl_ptc_close(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  if (pixel) {
    ptc_close();
    pixel = NULL;
  }
}

void gsl_ptc_update(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  if (pixel) {
    ptc_update(pixel);
  }
}

void gsl_ptc_set_pixel(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  int x   = GSL_LOCAL_INT(gsl,local,"x");
  int y   = GSL_LOCAL_INT(gsl,local,"y");
  int col = GSL_LOCAL_INT(gsl,local,"color");
  if ((x >= 0) && (y >= 0) && (x < width) && (y < height))
    pixel[x+y*width] = col;
  else
    fprintf(stderr, "Invalid Set Pixel (%d,%d)\n", x,y);
}

void gsl_ptc_get_pixel(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
  int x   = GSL_LOCAL_INT(gsl,local,"x");
  int y   = GSL_LOCAL_INT(gsl,local,"y");
  /* HashValue *ret = goom_hash_get(global,"ptc_get_pixel"); */
  GSL_GLOBAL_INT(gsl, "ptc_get_pixel") = pixel[x+y*width];
  /* *(int*)ret->ptr  = pixel[x+y*width]; */
}

void bind(GoomSL *gsl)
{
  /* Console functions */
  gsl_bind_function(gsl, "print", gsl_print);
  gsl_bind_function(gsl, "read_integer", gsl_read_integer);
  gsl_bind_function(gsl, "read_line", gsl_read_line);
  gsl_bind_function(gsl, "free", gsl_ext_free);
  gsl_bind_function(gsl, "random", gsl_random);
  gsl_bind_function(gsl, "srandom", gsl_srandom);
  gsl_bind_function(gsl, "strcmp", gsl_strcmp);

  /* graphical functions */
  gsl_bind_function(gsl, "ptc_open", gsl_ptc_open);
  gsl_bind_function(gsl, "ptc_close", gsl_ptc_close);
  gsl_bind_function(gsl, "ptc_set_pixel", gsl_ptc_set_pixel);
  gsl_bind_function(gsl, "ptc_get_pixel", gsl_ptc_get_pixel);
  gsl_bind_function(gsl, "ptc_update", gsl_ptc_update);
}

int main(int argc, char **argv)
{
    char *fbuffer;
    GoomSL *gsl;
    if (argc == 1) return 1;

    fbuffer = gsl_init_buffer(argv[argc-1]);
    gsl =  gsl_new();
    if (!gsl) return 1;
    gsl_compile(gsl,fbuffer);
    bind(gsl);
    gsl_execute(gsl);

    gsl_free(gsl);
    free(fbuffer);
    
    return 0;
}
