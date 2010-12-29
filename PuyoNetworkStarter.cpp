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

PuyoGame *PuyoNetworkGameFactory::createPuyoGame(PuyoFactory *attachedPuyoFactory) {
    return new PuyoNetworkGame(attachedPuyoFactory, msgBox, gameId);
}

PuyoNetworkGameWidget::PuyoNetworkGameWidget()
    : chatBox(NULL),
      brokenNetworkWidget(NULL), networkIsBroken(false),
      m_networkTimeoutWarning(5000.),
      m_networkTimeoutError(15000.)
{
}

PuyoPlayer *PuyoNetworkGameWidget::createLocalPlayer()
{
    return new PuyoCombinedEventPlayer(*localArea);
}

void PuyoNetworkGameWidget::initWithGUI(PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme, ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *gameOverAction, ios_fc::IgpMessageBox *igpbox)
{
    attachedPuyoThemeSet = &puyoThemeSet;
    attachedRandom = std::auto_ptr<PuyoRandomSystem>(new PuyoRandomSystem(randomSeed, 5));
    this->mbox = &mbox;
    attachedLocalGameFactory   = std::auto_ptr<PuyoLocalGameFactory>(new PuyoLocalGameFactory(attachedRandom.get()));
    attachedNetworkGameFactory = std::auto_ptr<PuyoNetworkGameFactory>(new PuyoNetworkGameFactory(attachedRandom.get(), mbox, gameId));
    if (igpbox != NULL) {
        localArea = std::auto_ptr<PuyoNetworkView>(new PuyoInternetNetworkView(attachedLocalGameFactory.get(), 0, attachedPuyoThemeSet, &levelTheme,
                                            &mbox, gameId, igpbox));
    }
    else {
        localArea = std::auto_ptr<PuyoNetworkView>(new PuyoNetworkView(attachedLocalGameFactory.get(), 0, attachedPuyoThemeSet, &levelTheme, &mbox, gameId));
    }
    networkArea = std::auto_ptr<PuyoView>(new PuyoView(attachedNetworkGameFactory.get(), 1, attachedPuyoThemeSet, &levelTheme));
    playercontroller = std::auto_ptr<PuyoPlayer>(createLocalPlayer());
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
    double curDate = ios_fc::getTimeMs();
    if (paused) {
        if (curDate - lastAliveMessageSentDate > 2000.) {
            sendAliveMsg();
            lastAliveMessageSentDate = curDate;
        }
    }
    // Management of network timeouts
    // (what to do when the network stops responding?)
    if (curDate - lastMessageDate > m_networkTimeoutWarning) {
        if (!networkIsBroken) {
            if (withGUI)
                associatedScreen->add(brokenNetworkWidget.get());
            networkIsBroken = true;
        }
        else if (curDate - lastMessageDate > m_networkTimeoutError) {
            // Call network failure action
            if (gameOverAction)
                gameOverAction->action(this, NETWORK_FAILURE, NULL);
        }
    }
    else if (networkIsBroken == true) {
        if (withGUI)
            associatedScreen->remove(brokenNetworkWidget.get());
        networkIsBroken = false;
    }
    // Let the game behave
    GameWidget::cycle();
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
        case PuyoMessage::kGamePause:
            setScreenToPaused(false);
            break;
        case PuyoMessage::kGameResume:
            setScreenToResumed(false);
            break;
        case PuyoMessage::kGameNext:
            actionAfterGameOver(false, GAMEOVER_STARTPRESSED);
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

void PuyoNetworkGameWidget::actionAfterGameOver(bool fromControls, int actionType)
{
    // If the resume is from a controller, we have to send the resume information to the other peer
    if (fromControls) {
        ios_fc::Message *message = mbox->createMessage();
        message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameNext);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    GameWidget::actionAfterGameOver(fromControls, actionType);
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

void PuyoNetworkGameWidget::sendAliveMsg()
{
    ios_fc::Message *message = mbox->createMessage();
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameAlive);
    message->send();
    delete message;
}

//---------------------------------
// NetSynchronizeState
//---------------------------------
NetSynchronizeState::NetSynchronizeState(ios_fc::MessageBox *mbox,
                                         int synID, double timeoutSec)
    : CycledComponent(0.1), m_mbox(mbox), m_synID(synID), m_cyclesTimeout(timeoutSec/0.1)
{
}

void NetSynchronizeState::enterState()
{
    cout << "NetSynchronizeState("<<m_synID<<")::enterState()" << endl;
    GameUIDefaults::GAME_LOOP->addIdle(this);
    m_synchronized = false;
    m_failed = false;
    m_ackSent = false;
    m_cyclesCounter = 0;
    m_mbox->addListener(this);
    // Send synchro message
    sendSyncMessage();
}

void NetSynchronizeState::exitState()
{
    GameUIDefaults::GAME_LOOP->removeIdle(this);
    m_mbox->removeListener(this);
    m_failed = false;
    m_ackSent = false;
}

bool NetSynchronizeState::evaluate()
{
    return ((m_synchronized && m_ackSent) || (m_failed));
}

GameState *NetSynchronizeState::getNextState()
{
    if (m_failed)
        return m_failedState;
    return m_nextState;
}

void NetSynchronizeState::onMessage(Message &message)
{
    if (!message.hasInt(PuyoMessage::TYPE))
        return;
    int msgType = message.getInt(PuyoMessage::TYPE);
    if ((msgType == PuyoMessage::kGameSync)
        || (msgType == PuyoMessage::kGameAck)){
        if (!message.hasInt("SynID"))
            return;
        int synID = message.getInt("SynID");
        if (synID == m_synID) {
            if (msgType == PuyoMessage::kGameSync) {
                if (!m_ackSent) {
                    sendAckMessage();
                    m_ackSent = true;
                }
                evaluateStateMachine();
            }
            else {
                m_synchronized = true;
                evaluateStateMachine();
            }
        }
    }
}

void NetSynchronizeState::cycle()
{
    if (m_cyclesCounter++ == m_cyclesTimeout) {
        m_failed = true;
        evaluateStateMachine();
    }
    sendSyncMessage();
}

void NetSynchronizeState::sendSyncMessage()
{
    auto_ptr<ios_fc::Message> message(m_mbox->createMessage());
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameSync);
    message->addInt     ("SynID", m_synID);
    message->send();
}

void NetSynchronizeState::sendAckMessage()
{
    auto_ptr<ios_fc::Message> message(m_mbox->createMessage());
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameAck);
    message->addInt     ("SynID", m_synID);
    message->addBoolProperty("RELIABLE", true);
    message->send();
}

//---------------------------------
// NetMatchPlayingState
//---------------------------------
NetMatchPlayingState::NetMatchPlayingState(SharedMatchAssets &sharedMatchAssets)
    : MatchPlayingState(sharedMatchAssets)
{
}

void NetMatchPlayingState::enterState()
{
    m_networkFailure = false;
    MatchPlayingState::enterState();
}

bool NetMatchPlayingState::evaluate()
{
    if (m_networkFailure)
        return true;
    return MatchPlayingState::evaluate();
}

GameState *NetMatchPlayingState::getNextState()
{
    if (m_networkFailure)
        return m_netfailState;
    return MatchPlayingState::getNextState();
}

void NetMatchPlayingState::action(Widget *sender, int actionType,
                                  event_manager::GameControlEvent *event)
{
    switch (actionType) {
        case PuyoNetworkGameWidget::NETWORK_FAILURE:
            m_networkFailure = true;
            evaluateStateMachine();
            break;
        default:
            MatchPlayingState::action(sender, actionType, event);
            break;
    }
}

//---------------------------------
// Two players network game state machine
//---------------------------------
NetworkGameStateMachine::NetworkGameStateMachine(GameWidgetFactory &gameWidgetFactory,
                                                 ios_fc::MessageBox *mbox,
                                                 int gameSpeed,
                                                 PuyoTwoNameProvider *nameProvider,
                                                 Action *endOfSessionAction)
{
    // Creating the different game states
    m_setupMatch.reset(new SetupMatchState(gameWidgetFactory, gameSpeed, nameProvider, m_sharedAssets));
    m_enterPlayersReady.reset(new EnterPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_synchroGetReady.reset(new NetSynchronizeState(mbox, 1));
    m_exitPlayersReady.reset(new ExitPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    //m_waitPlayersReady.reset(new WaitPlayersReadyState(m_sharedAssets));
    m_synchroBeforeStart.reset(new NetSynchronizeState(mbox, 2));
    m_matchPlaying.reset(new NetMatchPlayingState(m_sharedAssets));
    m_matchIsOver.reset(new MatchIsOverState(m_sharedAssets));
    m_displayStats.reset(new DisplayStatsState(m_sharedAssets));
    m_synchroAfterStats.reset(new NetSynchronizeState(mbox, 10));
    m_networkErrorScreen.reset(new DisplayStoryScreenState("netfailure.gsl"));
    m_leaveGame.reset(new LeaveGameState(m_sharedAssets, endOfSessionAction));

    // Linking the states together
    m_setupMatch->setNextState(m_enterPlayersReady.get());
    m_enterPlayersReady->setNextState(m_synchroGetReady.get());
    m_synchroGetReady->setNextState(m_exitPlayersReady.get());
    m_synchroGetReady->setFailedState(m_networkErrorScreen.get());
    m_exitPlayersReady->setNextState(m_synchroBeforeStart.get());
    m_synchroBeforeStart->setNextState(m_matchPlaying.get());
    m_synchroBeforeStart->setFailedState(m_networkErrorScreen.get());
    m_matchPlaying->setNextState(m_matchIsOver.get());
    m_matchPlaying->setAbortedState(m_leaveGame.get());
    m_matchPlaying->setNetworkFailedState(m_networkErrorScreen.get());
    m_matchIsOver->setNextState(m_displayStats.get());
    m_displayStats->setNextState(m_synchroAfterStats.get());
    m_synchroAfterStats->setNextState(m_setupMatch.get());
    m_synchroAfterStats->setFailedState(m_networkErrorScreen.get());
    m_networkErrorScreen->setNextState(m_leaveGame.get());

    // Initializing the state machine
    setInitialState(m_setupMatch.get());
}

