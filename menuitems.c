/* file created on Wed Aug 28 13:32:02 CEST 2002 by doom */

#include "menuitems.h"
#include <string.h>
#include <stdlib.h>

/* menu items */

void
menu_items_set_bitmap_for (MenuItems mi,
													 const char *name, SDL_Surface * bitmap)
{
	int     i = 0;

	while (mi[i].name) {
		if (!strcmp (mi[i].name, name)) {
			mi[i].bitmap = bitmap;
			return;
		}
		i++;
	}
}

SDL_Surface *
menu_items_get_bitmap (MenuItems mi, const char *name)
{
	int     i = 0;

	while (mi[i].name) {
		if (!strcmp (mi[i].name, name)) {
			return mi[i].bitmap;
		}
		i++;
	}
	return NULL;
}

void
menu_items_set_font_for (MenuItems mi, const char *name, SoFont * font)
{
	int     i = 0;

	while (mi[i].name) {
		if (!strcmp (mi[i].name, name)) {
			mi[i].font = font;
			return;
		}
		i++;
	}
}

SoFont *
menu_items_get_font (MenuItems mi, const char *name)
{
	int     i = 0;

	while (mi[i].name) {
		if (!strcmp (mi[i].name, name)) {
			return mi[i].font;
		}
		i++;
	}
	return NULL;
}

void
menu_items_set_value_for (MenuItems mi, const char *name, const char *value)
{
	int     i = 0;

	while (mi[i].name) {
		if (!strcmp (mi[i].name, name)) {
			if (mi[i].value)
				free (mi[i].value);
			if (value) {
				mi[i].value = (char *)malloc (strlen (value) + 1);
				strcpy (mi[i].value, value);
			}
			else
				mi[i].value = NULL;
			return;
		}
		i++;
	}
}

const char *
menu_items_get_value (MenuItems mi, const char *name)
{
	int     i = 0;

	while (mi[i].name) {
		if (!strcmp (mi[i].name, name)) {
			return mi[i].value;
		}
		i++;
	}
	return NULL;
}
