/* file created on Wed Aug 28 13:32:02 CEST 2002 by doom */

#ifndef _menuitems_H
#define _menuitems_H

/// MENU ITEMS

#include <SDL/SDL.h>
#include "sofont.h"

/// gestion des items du menu
/// ne jamais utiliser cette structure :
/// y'a des MACRO et des fonctions pour ca */
typedef struct _MENU_ITEM
{
	const char *name;
	char   *value;
	SDL_Surface *bitmap;					// NULL si c'est un separateur.
	SoFont *font;									// NULL si c'est un separateur.

	float   priv_vx;
	float   priv_x;
	int     priv_must_anim;

	int     inactive;
}
MenuItem;

typedef MenuItem *MenuItems;
typedef MenuItem MenuItemsTab[];

#define MENUITEM_BLANKLINE {"---",NULL,NULL,NULL,0,0,0,1}
#define MENUITEM_END {NULL,NULL,NULL,NULL,0,0,0,1}
#define MENUITEM(x) {x,NULL,NULL,NULL,0,0,0,0}
#define MENUITEM_INACTIVE(x) {x,NULL,NULL,NULL,0,0,0,1}

void    menu_items_set_bitmap_for (MenuItems mi,
																	 const char *name, SDL_Surface * bitmap);
SDL_Surface *menu_items_get_bitmap (MenuItems mi, const char *name);

void    menu_items_set_font_for (MenuItems mi,

																 const char *name, SoFont * font);
SoFont *menu_items_get_font (MenuItems mi, const char *name);

/// !!! une copie interne de value est créée !!!
void    menu_items_set_value_for (MenuItems mi, const char *name, const char *value);
const char *menu_items_get_value (MenuItems mi, const char *name);

#define  menu_items_is_delimiter(mi,index) (mi[index].inactive)


#endif /* _menuitems_H */
