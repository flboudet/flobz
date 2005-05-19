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

#include "PuyoLocalMenu.h"
#include "PuyoSinglePlayerStarter.h"

typedef enum SinglePlayerLevel {
    EASY,
    MEDIUM,
    HARD
} SinglePlayerLevel;

class SinglePlayerGameActionz : public Action {
public:
    SinglePlayerGameActionz(SinglePlayerLevel level) {;}
    void action();
};

/**
 * Launches a single player game
 */
void SinglePlayerGameActionz::action()
{
  PuyoStarter *starter = new PuyoSinglePlayerStarter(theCommander, 250, FLOBO, 0);
  starter->run(0,0,0,0,0);
  GameUIDefaults::SCREEN_STACK->push(starter);
}

void LocalGameMenu::build() {
    menu.add(new EditFieldWithLabel("Player Name:", "flobo"));
    menu.add(new Text("Choose Game Level"));
    menu.add(new Button("Easy", new SinglePlayerGameActionz(EASY)));
    menu.add(new Button("Medium", new SinglePlayerGameActionz(MEDIUM)));
    menu.add(new Button("Hard", new SinglePlayerGameActionz(HARD)));
    menu.add(new Button("Cancel", new PopScreenAction()));
    PuyoMainScreen::build();
}


void Local2PlayersGameMenu::build()
{
    menu.add(new EditFieldWithLabel("Player Name:", "flobo"));
    menu.add(new Text("Choose Game Level"));
    menu.add(new Button("Easy", new SinglePlayerGameActionz(EASY)));
    menu.add(new Button("Medium", new SinglePlayerGameActionz(MEDIUM)));
    menu.add(new Button("Hard", new SinglePlayerGameActionz(HARD)));
    menu.add(new Button("Cancel", new PopScreenAction()));
    PuyoMainScreen::build();
}

