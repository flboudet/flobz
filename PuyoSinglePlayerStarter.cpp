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

#include "PuyoSinglePlayerStarter.h"
#include "PuyoView.h"

extern IIM_Surface *perso[2];

PuyoSinglePlayerStarter::PuyoSinglePlayerStarter(PuyoCommander *commander, int aiLevel, IA_Type aiType, int theme)
: PuyoStarter(commander, theme)
{
    attachedGameFactory = new PuyoLocalGameFactory(&attachedRandom);
    areaA = new PuyoView(attachedGameFactory, &attachedThemeManager,
			     1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE);
    areaB = new PuyoView(attachedGameFactory, &attachedThemeManager,
			     1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE);
    
    
    attachedGameA = areaA->getAttachedGame();
    attachedGameB = areaB->getAttachedGame();
    
    randomPlayer = new PuyoIA(aiType, aiLevel, areaA);
    perso[0] = IIM_Load_DisplayFormatAlpha("perso1_1.png");
    perso[1] = IIM_Load_DisplayFormatAlpha("perso1_2.png");
    
    
    areaA->setEnemyGame(attachedGameB);
    areaB->setEnemyGame(attachedGameA);
}
