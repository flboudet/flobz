/*
 * This file is released under the GNU General Public Licence
 *
 * authors:
 * Richard Ashbury       <richard.asbury@btinternet.com>
 * Jean-Christophe Hoelt <jeko@ios-software.com>
 */

#include "corona32.h"
#include "corona.h"
#include "corona_palette.h"

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define WIDTH  400
#define HEIGHT 300

static const int PALETTEDATA[][NB_PALETTES] = {
#include "corona_pal.h"
};

/* Private members */

struct _Corona32
{
  Corona        *corona;
  PaletteCycler *pal;
  TimedLevel     tl;
  int            width;
  int            height;
};

/* Private methods */

void corona32_init_internals(Corona32 *_this)
{
  _this->corona       = new Corona();
  _this->pal          = new PaletteCycler(PALETTEDATA, NB_PALETTES);
  _this->tl.timeStamp = 0;
  _this->tl.lastbeat  = 0;
  _this->tl.state     = normal_state;
  _this->width        = 0;
  _this->height       = 0;
}

void corona32_free_internals(Corona32 *_this)
{
  delete _this->corona;
  delete _this->pal;
}

/* Public methods */

Corona32 *corona32_new(void)
{
  Corona32 *_this     = (Corona32*)calloc(1, sizeof(Corona32));
  corona32_init_internals(_this);
  return _this;
}

void corona32_delete(Corona32 *_this)
{
  corona32_free_internals(_this);
  free(_this);
}

void corona32_resize(Corona32 *_this, int width, int height)
{
  if (_this->width != 0) {
    corona32_free_internals(_this);
    corona32_init_internals(_this);
  }
  _this->corona->setUpSurface(width, height);
  _this->width  = width;
  _this->height = height;
}

void corona32_update(Corona32 *_this, int timeInMilli, short frequency[2][512])
{
  _this->tl.timeStamp ++;
  for (int i=0; i<512; ++i)
  {
    _this->tl.frequency[0][i] = frequency[0][i];
    _this->tl.frequency[1][i] = frequency[1][i];
  }
  _this->corona->update(&_this->tl); // Update Corona
  _this->pal->update(&_this->tl);    // Update Palette Cycler
}

void corona32_displayRGBA(Corona32 *_this, int *screen)
{
  int hwPal[256];
  paletteToRGBA(hwPal, _this->pal->getPalette());
  blitSurface8To32(_this->corona->getSurface(), screen, hwPal, _this->width * _this->height);
}

