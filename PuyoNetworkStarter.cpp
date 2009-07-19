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

PuyoNetworkGameWidget::PuyoNetworkGameWidget()
    : syncMsgReceived(false), syncMsgSent(false), chatBox(NULL),
      brokenNetworkWidget(NULL), networkIsBroken(false)
{
}

void PuyoNetworkGameWidget::initWithGUI(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *gameOverAction, ios_fc::IgpMessageBox *igpbox)
{
    attachedPuyoThemeSet = &puyoThemeSet;
    attachedRandom = std::auto_ptr<PuyoRandomSystem>(new PuyoRandomSystem(randomSeed, 5));
    this->mbox = &mbox;
    attachedLocalGameFactory   = std::auto_ptr<PuyoLocalGameFactory>(new PuyoLocalGameFactory(attachedRandom.get()));
    attachedNetworkGameFactory = std::auto_ptr<PuyoNetworkGameFactory>(new PuyoNetworkGameFactory(attachedRandom.get(), mbox, gameId));
    if (igpbox != NULL) {
        localArea = std::auto_ptr<PuyoNetworkView>(new PuyoInternetNetworkView(attachedLocalGameFactory.get(), attachedPuyoThemeSet, &levelTheme,
                                            1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE, &mbox, gameId, igpbox));
    }
    else {
        localArea = std::auto_ptr<PuyoNetworkView>(new PuyoNetworkView(attachedLocalGameFactory.get(), attachedPuyoThemeSet, &levelTheme,
                                    1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE, &mbox, gameId));
    }
    networkArea = std::auto_ptr<PuyoView>(new PuyoView(attachedNetworkGameFactory.get(), attachedPuyoThemeSet, &levelTheme,
                                                       1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE,
                                                       CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE));
    playercontroller = std::auto_ptr<PuyoCombinedEventPlayer>(new PuyoCombinedEventPlayer(*localArea));
    dummyPlayerController = std::auto_ptr<PuyoNullPlayer>(new PuyoNullPlayer(*networkArea));
    this->mbox->addListener(this);
    chatBox = std::auto_ptr<ChatBox>(new ChatBox(*this));
    brokenNetworkWidget = std::auto_ptr<StoryWidget>(new StoryWidget("etherdown.gsl"));
    networkIsBroken = false;
    GameWidget::initWithGUI(*localArea, *networkArea, *playercontroller, *dummyPlayerController, levelTheme, gameOverAction);
    setLives(-1);
}

void PuyoNetworkGameWidget::initWithoutGUI(ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *gameOverAction, ios_fc::IgpMessageBox *igpbox)
{
    this->mbox->addListener(this);
    //GameWidget::initWithoutGUI(*localArea, networkArea, playercontroller, dummyPlayerController, levelTheme, gameOverAction);
    setLives(-1);
}

void PuyoNetworkGameWidget::connectIA(int level)
{
}

PuyoNetworkGameWidget::~PuyoNetworkGameWidget()
{
    mbox->removeListener(this);
    // delete(localArea);
}

void PuyoNetworkGameWidget::cycle()
{
    mbox->idle();
    if (!syncMsgSent) {
        sendSyncMsg();
        syncMsgSent = true;
    }
    if (syncMsgReceived) {
        double curDate = ios_fc::getTimeMs();
        if (paused) {
            if (curDate - lastAliveMessageSentDate > 2000.) {
                sendAliveMsg();
                lastAliveMessageSentDate = curDate;
            }
        }
        if (curDate - lastMessageDate > 5000.) {
            if (!networkIsBroken) {
                if (withGUI)
                    associatedScreen->add(brokenNetworkWidget.get());
                networkIsBroken = true;
            }
            //printf("Network problem!\n");
        }
        else if (networkIsBroken == true) {
            if (withGUI)
                associatedScreen->remove(brokenNetworkWidget.get());
            networkIsBroken = false;
        }
        GameWidget::cycle();
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
            GameWidget::abort();
            break;
        case PuyoMessage::kGameChat:
            chatBox->addChat(message.getString("NAME"), message.getString("TEXT"));
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
        ios_fc::Message *message = mbox->createMessage();
        message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGamePause);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    lastAliveMessageSentDate = ios_fc::getTimeMs();
    GameWidget::setScreenToPaused(fromControls);
}

void PuyoNetworkGameWidget::setScreenToResumed(bool fromControls)
{
    // If the resume is from a controller, we have to send the resume information to the other peer
    if (fromControls) {
        ios_fc::Message *message = mbox->createMessage();
        message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameResume);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    GameWidget::setScreenToResumed(fromControls);
}

void PuyoNetworkGameWidget::abort()
{
    ios_fc::Message *message = mbox->createMessage();
    message->addInt(PuyoMessage::TYPE,   PuyoMessage::kGameAbort);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    delete message;
    GameWidget::abort();
}

void PuyoNetworkGameWidget::actionAfterGameOver(bool fromControls)
{
    // If the resume is from a controller, we have to send the resume information to the other peer
    if (fromControls) {
        ios_fc::Message *message = mbox->createMessage();
        message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameNext);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    GameWidget::actionAfterGameOver(fromControls);
}

void PuyoNetworkGameWidget::sendChat(String chatText)
{
    ios_fc::Message *message = mbox->createMessage();
    message->addInt(PuyoMessage::TYPE,   PuyoMessage::kGameChat);
    message->addString("NAME",   getPlayerOneName());
    message->addString("TEXT",   chatText);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    chatBox->addChat(getPlayerOneName(), chatText);
    delete message;
}

void PuyoNetworkGameWidget::associatedScreenHasBeenSet(GameScreen *associatedScreen)
{
    associatedScreen->getPauseMenu().add(chatBox.get());
    associatedScreen->getPauseMenu().pauseMenuTop = 5;
}

void PuyoNetworkGameWidget::sendSyncMsg()
{
    ios_fc::Message *message = mbox->createMessage();
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameStart);
    message->addString  (PuyoMessage::NAME,   p1name);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    delete message;
}

void PuyoNetworkGameWidget::sendAliveMsg()
{
    ios_fc::Message *message = mbox->createMessage();
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameAlive);
    message->send();
    delete message;
}



