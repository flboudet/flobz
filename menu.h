/* file created on Mon Aug 26 15:30:05 CEST 2002 by doom */

#ifndef _menu_H
#define _menu_H

#include "menuitems.h"
#include "audio.h"

/// MENU
///
/// gestionnaire de menu

typedef struct _MENU Menu;

///------------------------------
/// FONCTION DE PLUS HAUT NIVEAU 
///------------------------------

/// alloue un nouveau gestionnaire de menu
Menu   * menu_new (MenuItems items, SDL_Surface * selimage);

/// retourne 1 si l'item actif est it_name
int     menu_active_is (Menu * menu, const char *it_name);

void    menu_next_item (Menu * menu);
void    menu_prev_item (Menu * menu);

/// met a jour les donnees "temporelles" du menu..
/// (doit donc etre appele a chaque cycle du jeu/programme)
void    menu_update (Menu * menu, SDL_Surface * surf);

/// affiche le menu sur la surface
void    menu_draw (Menu * menu, SDL_Surface * surf);

/// null pour desactiver.
void    menu_set_sounds (Menu * menu, Sound * s_move, Sound * s_validate);

/// informe le menu que le choix a été fait..
/// (continuer d'appeler la methode d'affichage du menu..
/// il se chargera de ne rien faire si rien n'est à faire).
void    menu_validate (Menu * menu);

void    menu_show (Menu * menu);
void    menu_hide (Menu * menu);

int     menu_visible (Menu * menu);

/// switch la valeur d'un item ON/OFF
// retourne l'etat du switch
int     menu_switch_on_off (Menu * menu, const char *name);

int     menu_progress_bar (Menu * menu, const char *name, int increment);

///
/// Fonctions pour faire des choses vraiment particuliere
///

/// retourne l'index de l'item actif */
int     menu_get_active_item (Menu * menu);

/// retourne le nom de l'item actif */
const char *menu_get_active_item_string (Menu * menu);

/// donne la valeur de l'item name */
const char *menu_get_value (Menu * menu, const char *name);

/// affecte la valeur de l'item name */
void    menu_set_value (Menu * menu, const char *name, const char *value);

#endif /* _menu_H */
