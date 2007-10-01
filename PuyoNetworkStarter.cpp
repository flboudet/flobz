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
#include "ios_time.h"
#include "PuyoMessageDef.h"

extern const char *p1name;
extern const char *p2name;

extern IIM_Surface *perso[2];

PuyoGame *PuyoNetworkGameFactory::createPuyoGame(PuyoFactory *attachedPuyoFactory) {
    return new PuyoNetworkGame(attachedPuyoFactory, msgBox, gameId);
}

PuyoNetworkGameWidget::PuyoNetworkGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *gameOverAction)
    : attachedPuyoThemeSet(puyoThemeSet), attachedRandom(randomSeed, 5), mbox(mbox), attachedLocalGameFactory(&attachedRandom),
      attachedNetworkGameFactory(&attachedRandom, mbox, gameId), localArea(&attachedLocalGameFactory, &attachedPuyoThemeSet, &levelTheme,
            1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE, &mbox, gameId, painter),
      networkArea(&attachedNetworkGameFactory, &attachedPuyoThemeSet, &levelTheme,
            1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE, painter),
      playercontroller(localArea), dummyPlayerController(networkArea), syncMsgReceived(false), syncMsgSent(false), chatBox(*this),
      brokenNetworkWidget("etherdown.gsl"), networkIsBroken(false)
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
    if (!syncMsgSent) {
        sendSyncMsg();
        syncMsgSent = true;
    }
    if (syncMsgReceived) {
        PuyoGameWidget::cycle();
        double curDate = ios_fc::getTimeMs();
        if (paused) {
            if (curDate - lastAliveMessageSentDate > 2000.) {
                sendAliveMsg();
                lastAliveMessageSentDate = curDate;
            }
        }
        if (curDate - lastMessageDate > 5000.) {
            if (!networkIsBroken) {
                associatedScreen->add(&brokenNetworkWidget);
                networkIsBroken = true;
            }
            //printf("Network problem!\n");
        }
        else if (networkIsBroken == true) {
            associatedScreen->remove(&brokenNetworkWidget);
            networkIsBroken = false;
        }
    }
}

void PuyoNetworkGameWidget::onMessage(Message &message)
{
    if (message.hasInt(PuyoMessage::GAMEID)) {
        lastMessageDate = ios_fc::getTimeMs();
    }
    if (!message.hasInt(PuyoMessage::TYPE))
        return;
    lastMessageDate = ios_fc::getTimeMs();
    printf("Message at date %f\n", lastMessageDate);
    int msgType = message.getInt(PuyoMessage::TYPE);
    switch (msgType) {
        case PuyoMessage::kGameStart:
            syncMsgReceived = true;
            break;
        case PuyoMessage::kGamePause:
            setScreenToPaused(false);
            break;
        case PuyoMessage::kGameResume:
            setScreenToResumed(false);
            break;
        case PuyoMessage::kGameNext:
            actionAfterGameOver(false);
            break;
        case PuyoMessage::kGameAbort:
            PuyoGameWidget::abort();
            break;
        case PuyoMessage::kGameChat:
            chatBox.addChat(message.getString("NAME"), message.getString("TEXT"));
            printf("%s: %s\n", (const char *)message.getString("NAME"), (const char *)message.getString("TEXT"));
            break;
        default:
            break;
    }
}

void PuyoNetworkGameWidget::setScreenToPaused(bool fromControls)
{
    // If the pause is from a controller, we have to send the pause information to the other peer
    if (fromControls) {
        ios_fc::Message *message = mbox.createMessage();
        message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGamePause);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    lastAliveMessageSentDate = ios_fc::getTimeMs();
    PuyoGameWidget::setScreenToPaused(fromControls);
}

void PuyoNetworkGameWidget::setScreenToResumed(bool fromControls)
{
    // If the resume is from a controller, we have to send the resume information to the other peer
    if (fromControls) {
        ios_fc::Message *message = mbox.createMessage();
        message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameResume);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    PuyoGameWidget::setScreenToResumed(fromControls);
}

void PuyoNetworkGameWidget::abort()
{
    ios_fc::Message *message = mbox.createMessage();
    message->addInt(PuyoMessage::TYPE,   PuyoMessage::kGameAbort);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    delete message;
    PuyoGameWidget::abort();
}

void PuyoNetworkGameWidget::actionAfterGameOver(bool fromControls)
{
    fprintf(stderr, "ACTIONAFTERGAMEOVER\n");
    // If the resume is from a controller, we have to send the resume information to the other peer
    if (fromControls) {
        ios_fc::Message *message = mbox.createMessage();
        message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameNext);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    PuyoGameWidget::actionAfterGameOver(fromControls);
}

void PuyoNetworkGameWidget::sendChat(String chatText)
{
    ios_fc::Message *message = mbox.createMessage();
    message->addInt(PuyoMessage::TYPE,   PuyoMessage::kGameChat);
    message->addString("NAME",   getPlayerOneName());
    message->addString("TEXT",   chatText);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    chatBox.addChat(getPlayerOneName(), chatText);
    delete message;
}

void PuyoNetworkGameWidget::associatedScreenHasBeenSet(PuyoGameScreen *associatedScreen)
{
    associatedScreen->getPauseMenu().add(&chatBox);
    associatedScreen->getPauseMenu().pauseMenuTop = 5;
    //associatedScreen->getPauseMenu().arrangeWidgets();
}

void PuyoNetworkGameWidget::sendSyncMsg()
{
    ios_fc::Message *message = mbox.createMessage();
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameStart);
    message->addString  (PuyoMessage::NAME,   p1name);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    delete message;
}

void PuyoNetworkGameWidget::sendAliveMsg()
{
    ios_fc::Message *message = mbox.createMessage();
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameAlive);
    message->send();
    delete message;
}



