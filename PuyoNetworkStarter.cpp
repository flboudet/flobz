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

#include "PuyoNetworkStarter.h"
#include "PuyoMessageDef.h"

extern const char *p1name;
extern const char *p2name;

PuyoNetworkStarter::PuyoNetworkStarter(PuyoCommander *commander, int theme, ios_fc::MessageBox *mbox)
: PuyoStarter(commander, false, 0, RANDOM, theme, mbox)
{
    netgame_started = false;
    mbox->addListener(this);
}

void PuyoNetworkStarter::run(int score1, int score2, int lives, int point1, int point2)
{
    // Network game startup synchronization
    ios_fc::Message *message = mbox->createMessage();
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameStart);
    message->addString  (PuyoMessage::NAME,   p1name);
    message->send();
    delete message;
    
    while ((!netgame_started) && (!gameAborted)) {
        SDL_Event event;
        while (SDL_PollEvent(&event) == 1) {
            handleEvent(event);
        }
        commander->updateAll(this);
    }
    if (!gameAborted)
        PuyoStarter::run(score1, score2, lives, point1, point2);
}

void PuyoNetworkStarter::backPressed()
{
    if (netgame_started) {
        PuyoStarter::backPressed();
    }
    else {
        gameAborted = true;
    }
}

void PuyoNetworkStarter::onMessage(Message &message)
{
    int msgType = message.getInt(PuyoMessage::TYPE);
    switch (msgType) {
        case PuyoMessage::kGameStart:
 	    netgame_started = true;
            break;
        default:
            break;
    }
}
