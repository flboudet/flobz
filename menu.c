/* file created on Mon Aug 26 15:30:05 CEST 2002 by doom */

#include "menu.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>


struct _MENU
{
  MenuItems items;
  int     active_item;
  int     nb_items;
  IIM_Surface * menuselector;
  
  int     sep_size;

  Sound  *sound_move;
  Sound  *sound_valid;

  int     appears;
  int     visible;
  int     hide;

  int     hauteur;
};


void    menu_update_hauteur (Menu * menu);


  Menu   *
menu_new (MenuItems items,IIM_Surface * selimage)
{
  Menu   *m = (Menu *)malloc (sizeof (Menu));

  m->items = items;

  m->nb_items = 0;
  while (items[m->nb_items++].name);

  m->sep_size = 20;							// taille d'un separateur.

  m->active_item = -1;
  m->sound_move = m->sound_valid = NULL;

  m->visible = m->appears = m->hide = 0;
  menu_update_hauteur (m);

  menu_next_item (m);

  
  m->menuselector = selimage;;
      
  return m;
}


int menu_get_active_item (Menu * menu)
{
  return menu->active_item;
}

const char * menu_get_active_item_string (Menu * menu)
{
  return menu->items[menu_get_active_item (menu)].name;
}

int menu_active_is (Menu * menu, const char *it_name)
{
  return (!strcmp (it_name, menu_get_active_item_string (menu)));
}


void menu_change_item_done (Menu * menu)
{
  menu->items[menu->active_item].priv_must_anim = 1;
  if (menu->sound_move) {
    audio_sound_play (menu->sound_move);
  }
}


  void
menu_next_item (Menu * menu)
{
  do {
    menu->active_item++;
    menu->active_item %= menu->nb_items;
  }
  while (menu_items_is_delimiter (menu->items, menu->active_item));
  menu_change_item_done (menu);
}

  void
menu_prev_item (Menu * menu)
{
  do {
    if (menu->active_item)
      menu->active_item--;
    else
      menu->active_item = menu->nb_items - 1;
  }
  while (menu_items_is_delimiter (menu->items, menu->active_item));
  menu_change_item_done (menu);
}


void menu_update (Menu * menu, SDL_Surface * surf)
{
  int     c;

  if (menu->visible) {
    if (menu->hide) {
      menu->hide *= 2;
    }

    if (menu->hide > surf->h) {
      menu->visible = 0;
      menu->hide = 0;
    }

    if (menu->appears) {
      for (c = 0; c < menu->nb_items; c++) {
        menu->items[c].priv_vx = 0;
        menu->items[c].priv_x =
          ((rand () % 2) ? -1 : 1) * (16 + (rand () % 4)) * surf->w / 16;
      }
      menu->appears = 0;
    }

    for (c = 0; c < menu->nb_items; c++) {
      SDL_Surface *item_surf = menu->items[c].bitmap;
      SoFont *item_font = menu->items[c].font;
     
      if (item_surf || item_font) {
        if (menu->items[c].priv_must_anim) {
          static int alt = -1;

          menu->items[c].priv_vx += alt * 15;
          menu->items[c].priv_must_anim = 0;
          alt = -alt;
        }

        if ((fabs (menu->items[c].priv_vx) < 0.02)
            && (fabs (menu->items[c].priv_x) < 0.1))
          menu->items[c].priv_x = menu->items[c].priv_vx = 0;

        menu->items[c].priv_vx -= menu->items[c].priv_x / 8;
        menu->items[c].priv_vx *= 0.85;

        menu->items[c].priv_x += menu->items[c].priv_vx;
      }
    }
  }
}

void menu_draw (Menu * menu, SDL_Surface * surf)
{
  if (menu->visible) {
    int     c;									// compteur d'items
    int     center = surf->w / 2;
    int     hauteur = (surf->h - menu->hauteur) / 2;

    hauteur += menu->hide;

    for (c = 0; c < menu->nb_items; c++) {
      SDL_Surface *item_surf = menu->items[c].bitmap;
      SoFont *item_font = menu->items[c].font;

      if (item_surf || item_font) {
        int     center_x;

        center_x = (int)(center + menu->items[c].priv_x);

        if (item_surf != NULL) {
          SDL_Rect rect;

          rect.x = center_x - item_surf->w / 2;
          rect.y = hauteur;
          rect.w = item_surf->w;
          rect.h = item_surf->h;

          SDL_BlitSurface (item_surf, NULL, surf, &rect);
          hauteur += item_surf->h;
        }
        else {									// font..
          static char item_name[1024];

          if (c == menu->active_item) {
              
            SDL_Rect rect;
            if (menu->menuselector==NULL)
            {
              rect.x = center - 140; rect.w = 280;
              rect.y = hauteur; rect.h = SoFont_FontHeight (item_font);
              SDL_FillRect (surf, &rect, 0);
              rect.x = center - 150; rect.w = 300;
              rect.y = hauteur + 3; rect.h = SoFont_FontHeight (item_font) - 6;
              SDL_FillRect (surf, &rect, 0);
            }
            else
            {
              rect.x = center - 150; rect.w = menu->menuselector->w;
              rect.y = hauteur - 3; rect.h = menu->menuselector->h;
              SDL_BlitSurface (menu->menuselector->surf, NULL, surf, &rect);
            }
          }

          strcpy (item_name, menu->items[c].name);
          if (menu->items[c].value) {
            strcat (item_name, menu->items[c].value);
          }
          
          char *pc = item_name;
          int nbTab = 0;
          for (; *pc; pc++)
          {
            if (*pc == '\t') {
              while (*pc == '\t') {
                *pc = 0;
                pc    ++;
                nbTab ++;
              }
              break;
            }
          }
          
          if (nbTab > 0)
          {
            int w = 80 + nbTab * 50;
            SoFont_PutString (item_font, surf, center_x-w, hauteur, item_name, NULL);
            SoFont_PutString (item_font, surf, center_x+w - SoFont_TextWidth(item_font, pc), hauteur, pc, NULL);
          }
          else
          {
            SoFont_PutString (item_font, surf,
                center_x - SoFont_TextWidth (item_font,
                  item_name) / 2,
                hauteur, item_name, NULL);
          }
          hauteur += SoFont_FontHeight (item_font);
        }
      }
      else
        hauteur += menu->sep_size;
    }
  }
}

void menu_update_hauteur (Menu * menu)
{
  int     c;										// compteur d'items
  int     hauteur = 0;

  for (c = 0; c < menu->nb_items; c++) {
    SDL_Surface *item_surf = menu->items[c].bitmap;
    SoFont *item_font = menu->items[c].font;

    if (item_surf || item_font) {
      if (item_surf != NULL) {
        hauteur += item_surf->h;
      }
      else {										// font..
        hauteur += SoFont_FontHeight (item_font);
      }
    }
    else
      hauteur += menu->sep_size;
  }
  menu->hauteur = hauteur;
}


  void
menu_set_sounds (Menu * menu, Sound * s_move, Sound * s_validate)
{
  menu->sound_move = s_move;
  menu->sound_valid = s_validate;
}

  void
menu_validate (Menu * menu)
{
  if (menu->sound_valid)
    audio_sound_play (menu->sound_valid);
  menu->items[menu->active_item].priv_must_anim = 1;
}

  void
menu_show (Menu * menu)
{
  if (!menu_visible (menu)) {
    menu->appears = 1;
    menu->visible = 1;
    menu->hide = 0;
  }
}

  void
menu_hide (Menu * menu)
{
  if (menu_visible (menu))
    menu->hide = 1;
}

  int
menu_visible (Menu * menu)
{
  return (menu->visible) && (!menu->hide);
}

  int
menu_switch_on_off (Menu * menu, const char *name)
{
  const char *value = menu_items_get_value (menu->items, name);

  if (!strcmp (value, "ON")) {
    menu_set_value (menu, name, "OFF");
    return 0;
  }
  else {
    menu_set_value (menu, name, "ON");
    return 1;
  }
}

  int
menu_progress_bar (Menu * menu, const char *name, int increment)
{
  const char *svalue = menu_items_get_value (menu->items, name);
  int     value = atoi (svalue);
  static char nval[5];

  if (increment == 0)
    return value;
  else {
    value += increment;
    if (value > 100)
      value = 100;
    if (value < 0)
      value = 0;
  }

  sprintf (nval, "%d %%", value);
  menu_set_value (menu, name, nval);
  return value;
}


  const char *
menu_get_value (Menu * menu, const char *name)
{
  return menu_items_get_value (menu->items, name);
}

  void
menu_set_value (Menu * menu, const char *name, const char *value, int doValide)
{
  menu_items_set_value_for (menu->items, name, value);
  if (doValide)
    menu_validate (menu);
}
