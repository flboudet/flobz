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
#include "PuyoNetworkView.h"
#include "PuyoNetworkGame.h"

extern const char *p1name;
extern const char *p2name;

extern IIM_Surface *perso[2];

PuyoGame *PuyoNetworkGameFactory::createPuyoGame(PuyoFactory *attachedPuyoFactory) {
    return new PuyoNetworkGame(attachedPuyoFactory, msgBox);
}

PuyoNetworkGameWidget::PuyoNetworkGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, ios_fc::MessageBox &mbox, Action *gameOverAction)
    : attachedPuyoThemeSet(puyoThemeSet), mbox(mbox), attachedLocalGameFactory(&attachedRandom),
      attachedNetworkGameFactory(&attachedRandom, mbox), localArea(&attachedLocalGameFactory, &attachedPuyoThemeSet, &levelTheme,
            1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE, &mbox, painter),
      networkArea(&attachedNetworkGameFactory, &attachedPuyoThemeSet, &levelTheme,
            1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE, painter),
      playercontroller(localArea), dummyPlayerController(networkArea)
{
    mbox.addListener(this);
    initialize(localArea, networkArea, playercontroller, dummyPlayerController, levelTheme, gameOverAction);
    setLives(-1);
}

PuyoNetworkGameWidget::~PuyoNetworkGameWidget()
{
    mbox.removeListener(this);
}

void PuyoNetworkGameWidget::cycle()
{
    mbox.idle();
    PuyoGameWidget::cycle();
}

void PuyoNetworkGameWidget::onMessage(Message &)
{
}

NetworkStarterAction::NetworkStarterAction(String _IP)
    : IP(_IP), difficulty(1), gameScreen(NULL), gameWidget(NULL)
{
    mbox = NULL;
}

NetworkStarterAction::~NetworkStarterAction()
{
    if (mbox != NULL) delete mbox;
}

void NetworkStarterAction::action()
{
    if (gameScreen == NULL) {
        startGame();
    }
    else if (! gameWidget->getAborted()) {
        gameOver();
    }
    else {
	    endGameSession();
	}
}

void NetworkStarterAction::startGame()
{
    AnimatedPuyoThemeManager * themeManager = getPuyoThemeManger();
    
    if (mbox == NULL) mbox = new UDPMessageBox((const char *)IP, 6581, 6581);
    
    gameWidget = new PuyoNetworkGameWidget(*(themeManager->getAnimatedPuyoSetTheme()), *(themeManager->getPuyoLevelTheme()), *mbox, this);
    gameScreen = new PuyoGameScreen(*gameWidget, *(GameUIDefaults::SCREEN_STACK->top()));
    /*if (nameProvider != NULL) {
        gameWidget->setPlayerOneName(nameProvider->getPlayer1Name());
        gameWidget->setPlayerTwoName(nameProvider->getPlayer2Name());
    }*/
    GameUIDefaults::SCREEN_STACK->push(gameScreen);
}

void NetworkStarterAction::gameOver()
{
    GameUIDefaults::SCREEN_STACK->pop();
    delete gameWidget;
    delete gameScreen;
    
    gameScreen = NULL;
    gameWidget = NULL;
    startGame();
}

void NetworkStarterAction::endGameSession()
{
    GameUIDefaults::SCREEN_STACK->pop();
    ((PuyoRealMainScreen *)(GameUIDefaults::SCREEN_STACK->top()))->transitionFromScreen(*gameScreen);
    delete gameWidget;
    delete gameScreen;
    
    gameScreen = NULL;
    gameWidget = NULL;
}


#ifdef DESACTIVE

PuyoNetworkStarter::PuyoNetworkStarter(PuyoCommander *commander, int theme, ios_fc::MessageBox *mbox)
: PuyoStarter(commander, theme), mbox(mbox)
{
    attachedGameFactory = new PuyoLocalGameFactory(&attachedRandom);
    attachedNetworkGameFactory = new PuyoNetworkGameFactory(&attachedRandom, *mbox);
    areaA = new PuyoView(attachedNetworkGameFactory, &attachedThemeManager,
                         1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE);
    areaB = new PuyoNetworkView(attachedGameFactory, &attachedThemeManager,
                                1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE, mbox);
    //SDL_Delay(10000);
    
    
    attachedGameA = areaA->getAttachedGame();
    attachedGameB = areaB->getAttachedGame();
    
    randomPlayer = 0;
    perso[0] = NULL;
    
    areaA->setEnemyGame(attachedGameB);
    areaB->setEnemyGame(attachedGameA);

    netgame_started = false;
    mbox->addListener(this);
}

/*
void PuyoNetworkStarter::run(int score1, int score2, int lives, int point1, int point2)
{
    // Network game startup synchronization
    ios_fc::Message *message = mbox->createMessage();
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameStart);
    message->addString  (PuyoMessage::NAME,   p1name);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    
    while ((!netgame_started) && (!gameAborted)) {
        SDL_Event event;
        while (SDL_PollEvent(&event) == 1) {
            handleEvent(event);
        }
        mbox->idle();
        commander->updateAll(this);
    }
    if (!gameAborted)
        // We send the message twice
        message->send();
        delete message;
        PuyoStarter::run(score1, score2, lives, point1, point2);
}*/

void PuyoNetworkStarter::cycle()
{
    static bool once = true;
    static bool once_started = true;
    if (once) {
        once = false;
        printf("Message de synchro!\n");
        // Network game startup synchronization
        ios_fc::Message *message = mbox->createMessage();
        message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameStart);
        message->addString  (PuyoMessage::NAME,   p1name);
        message->addBoolProperty("RELIABLE", true);
        message->send();
    }
    
    if (mbox != NULL)
        mbox->idle();
    
    if (netgame_started) {
        if (once_started) {
            // We send the message twice
            printf("Message de synchro 2!\n");
            ios_fc::Message *message = mbox->createMessage();
            message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameStart);
            message->addString  (PuyoMessage::NAME,   p1name);
            message->addBoolProperty("RELIABLE", true);
            message->send();
            once_started = false;
        }
        PuyoStarter::cycle();
    }
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
    try {
        int msgType = message.getInt(PuyoMessage::TYPE);
        switch (msgType) {
            case PuyoMessage::kGameStart:
                netgame_started = true;
                break;
            default:
                break;
        }
    }
    catch (Exception e) {
        printf("Message invalide!\n");
    }
}

#endif // DESACTIVE