/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gyom@ios-software.com>
 *
 * iOS Software <http://www.ios-software.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */
 
#ifndef _PUYOIGPDEFS_H
#define _PUYOIGPDEFS_H

enum {
    PUYO_IGP_ALIVE,
    PUYO_IGP_CONNECT,
    PUYO_IGP_DISCONNECT,
    PUYO_IGP_CHAT,
    PUYO_IGP_GAME_REQUEST,
    PUYO_IGP_GAME_ACCEPT,
    PUYO_IGP_GAME_CANCEL,
    PUYO_IGP_STATUSCHANGE,
    PUYO_IGP_NAT_TRAVERSAL,
    PUYO_IGP_NAT_TRAVERSAL_GARBAGE,
    PUYO_IGP_NAT_TRAVERSAL_SYNC
};

#endif // _PUYOIGPDEFS_H

