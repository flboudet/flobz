#ifndef _CORONA_32
#define _CORONA_32

/*
 * This file is released under the GNU General Public Licence
 *
 * authors:
 * Richard Ashbury       <richard.asbury@btinternet.com>
 * Jean-Christophe Hoelt <jeko@ios-software.com>
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Corona32 Corona32;

Corona32 *corona32_new(void);
void      corona32_delete(Corona32 *_this);

void      corona32_resize(Corona32 *_this, int widht, int height);
void      corona32_update(Corona32 *_this, int timeInMilli, short frequency[2][512]);
void      corona32_displayRGBA(Corona32 *_this, int *screen);

#ifdef __cplusplus
};
#endif

#endif
