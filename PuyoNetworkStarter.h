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
#ifndef _PUYONETWORKSTARTER
#define _PUYONETWORKSTARTER

#include "PuyoStarter.h"
#include "PuyoSinglePlayerStarter.h"
#include "ios_messagebox.h"
#include "PuyoNetworkView.h"
#include "iosfc/ios_memory.h"

class PuyoNetworkGameFactory : public PuyoGameFactory {
public:
    PuyoNetworkGameFactory(PuyoRandomSystem *attachedRandom, MessageBox &msgBox): attachedRandom(attachedRandom), msgBox(msgBox) {}
    PuyoGame *createPuyoGame(PuyoFactory *attachedPuyoFactory);
private:
    PuyoRandomSystem *attachedRandom;
    MessageBox &msgBox;
};

class PuyoNetworkGameWidget : public PuyoGameWidget, MessageListener {
public:
    PuyoNetworkGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, ios_fc::MessageBox &mbox, Action *gameOverAction = NULL);
    ~PuyoNetworkGameWidget();
    bool didPlayerWon() const { return isGameARunning(); }
    void cycle();
    void onMessage(Message &);
    void setScreenToPaused(bool fromControls);
    void setScreenToResumed(bool fromControls);
private:
    void sendSyncMsg();
    AnimatedPuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    ios_fc::MessageBox &mbox;
    PuyoLocalGameFactory attachedLocalGameFactory;
    PuyoNetworkGameFactory attachedNetworkGameFactory;
    PuyoNetworkView localArea;
    PuyoView networkArea;
    PuyoCombinedEventPlayer playercontroller;
    PuyoNullPlayer dummyPlayerController;
    bool syncMsgReceived, syncMsgSent;
};

class NetworkStarterAction : public Action {
public:
    NetworkStarterAction(String _IP);
    ~NetworkStarterAction();
    void action();
    
private:
    String IP;
    ios_fc::MessageBox * mbox;
    void startGame();
    void gameOver();
    void endGameSession();
    
    int difficulty;
    PuyoGameScreen *gameScreen;
    PuyoNetworkGameWidget *gameWidget;
    //PuyoTwoNameProvider *nameProvider;
};

#endif // _PUYONETWORKSTARTER
