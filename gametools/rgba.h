#ifndef _RGBA_H_
#define _RGBA_H_

#include <stdint.h>

struct RGBA {
  RGBA(uint8_t red=0, uint8_t green=0, uint8_t blue=0, uint8_t alpha=0) :red(red),green(green),blue(blue),alpha(alpha) {}
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
};

typedef struct _HSVA {
  float hue;
  float saturation;
  float value;
  uint8_t alpha;
} HSVA;

extern RGBA GT_WHITE;
extern RGBA GT_BLACK;

#endif /* _RGBA_H_ */

