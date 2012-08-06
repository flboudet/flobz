/* FloboPop
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

#include "NetworkGameStarter.h"
#include "FPNetMessageDef.h"
#include "NetworkGameView.h"
#include "NetworkGame.h"
#include "ios_time.h"
#include "FPNetMessageDef.h"

FloboGame *NetworkGameFactory::createFloboGame(FloboFactory *attachedFloboFactory) {
    return new NetworkGame(attachedFloboFactory, &msgBox, gameId);
}

NetworkGameWidget::NetworkGameWidget()
    : chatBox(NULL),
      brokenNetworkWidget(NULL), networkIsBroken(false),
      m_networkTimeoutWarning(5000.),
      m_networkTimeoutError(15000.)
{
}

GamePlayer *NetworkGameWidget::createLocalPlayer()
{
    return new CombinedEventPlayer(*localArea);
}

void NetworkGameWidget::initWithGUI(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *gameOverAction, FPServerIGPMessageBox *igpbox)
{
    attachedFloboThemeSet = &floboSetTheme;
    attachedRandom = std::auto_ptr<RandomSystem>(new RandomSystem(randomSeed, 5));
    this->mbox = &mbox;
    attachedLocalGameFactory   = std::auto_ptr<LocalGameFactory>(new LocalGameFactory(attachedRandom.get()));
    attachedNetworkGameFactory = std::auto_ptr<NetworkGameFactory>(new NetworkGameFactory(attachedRandom.get(), mbox, gameId));
    if (igpbox != NULL) {
        localArea = std::auto_ptr<NetworkGameView>(new InternetGameView(attachedLocalGameFactory.get(), 0, attachedFloboThemeSet, &levelTheme,
                                            &mbox, gameId, igpbox));
    }
    else {
        localArea = std::auto_ptr<NetworkGameView>(new NetworkGameView(attachedLocalGameFactory.get(), 0, attachedFloboThemeSet, &levelTheme, &mbox, gameId));
    }
    networkArea = std::auto_ptr<GameView>(new GameView(attachedNetworkGameFactory.get(), 1, attachedFloboThemeSet, &levelTheme));
    playercontroller = std::auto_ptr<GamePlayer>(createLocalPlayer());
    dummyPlayerController = std::auto_ptr<GameNullPlayer>(new GameNullPlayer(*networkArea));
    this->mbox->addListener(this);
    chatBox = std::auto_ptr<ChatBox>(new ChatBox(*this));
    brokenNetworkWidget = std::auto_ptr<StoryWidget>(new StoryWidget("etherdown.gsl"));
    networkIsBroken = false;
    GameWidget2P::initWithGUI(*localArea, *networkArea, *playercontroller, *dummyPlayerController, levelTheme, gameOverAction);
    setLives(-1);
}

void NetworkGameWidget::initWithoutGUI(ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *gameOverAction, FPServerIGPMessageBox *igpbox)
{
    this->mbox->addListener(this);
    //GameWidget::initWithoutGUI(*localArea, networkArea, playercontroller, dummyPlayerController, levelTheme, gameOverAction);
    setLives(-1);
}

void NetworkGameWidget::connectIA(int level)
{
}

NetworkGameWidget::~NetworkGameWidget()
{
    mbox->removeListener(this);
    // delete(localArea);
}

void NetworkGameWidget::cycle()
{
    double curDate = ios_fc::getTimeMs();
    if (m_paused) {
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
    GameWidget2P::cycle();
    mbox->idle();
}

void NetworkGameWidget::onMessage(Message &message)
{
    if (message.hasInt(FPNetMessage::GAMEID)) {
        lastMessageDate = ios_fc::getTimeMs();
    }
    if (!message.hasInt(FPNetMessage::TYPE))
        return;
    lastMessageDate = ios_fc::getTimeMs();
    int msgType = message.getInt(FPNetMessage::TYPE);
    switch (msgType) {
        case FPNetMessage::kGamePause:
            setScreenToPaused(false);
            break;
        case FPNetMessage::kGameResume:
            setScreenToResumed(false);
            break;
        case FPNetMessage::kGameNext:
            actionAfterGameOver(false, GAMEOVER_STARTPRESSED);
            break;
        case FPNetMessage::kGameAbort:
            GameWidget2P::abort();
            break;
        case FPNetMessage::kGameChat:
            chatBox->addChat(message.getString("NAME"), message.getString("TEXT"));
            printf("%s: %s\n", (const char *)message.getString("NAME"), (const char *)message.getString("TEXT"));
            break;
        default:
            break;
    }
}

void NetworkGameWidget::setScreenToPaused(bool fromControls)
{
    // If the pause is from a controller, we have to send the pause information to the other peer
    if (fromControls) {
        ios_fc::Message *message = mbox->createMessage();
        message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGamePause);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    lastAliveMessageSentDate = ios_fc::getTimeMs();
    GameWidget2P::setScreenToPaused(fromControls);
}

void NetworkGameWidget::setScreenToResumed(bool fromControls)
{
    // If the resume is from a controller, we have to send the resume information to the other peer
    if (fromControls) {
        ios_fc::Message *message = mbox->createMessage();
        message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameResume);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    GameWidget2P::setScreenToResumed(fromControls);
}

void NetworkGameWidget::abort()
{
    ios_fc::Message *message = mbox->createMessage();
    message->addInt(FPNetMessage::TYPE,   FPNetMessage::kGameAbort);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    delete message;
    GameWidget2P::abort();
}

void NetworkGameWidget::actionAfterGameOver(bool fromControls, int actionType)
{
    // If the resume is from a controller, we have to send the resume information to the other peer
    if (fromControls) {
        ios_fc::Message *message = mbox->createMessage();
        message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameNext);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    GameWidget2P::actionAfterGameOver(fromControls, actionType);
}

void NetworkGameWidget::sendChat(String chatText)
{
    ios_fc::Message *message = mbox->createMessage();
    message->addInt(FPNetMessage::TYPE,   FPNetMessage::kGameChat);
    message->addString("NAME",   getPlayerOneName());
    message->addString("TEXT",   chatText);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    chatBox->addChat(getPlayerOneName(), chatText);
    delete message;
}

void NetworkGameWidget::associatedScreenHasBeenSet(GameScreen *associatedScreen)
{
    associatedScreen->getPauseMenu().add(chatBox.get());
    associatedScreen->getPauseMenu().pauseMenuTop = 5;
}

void NetworkGameWidget::sendAliveMsg()
{
    ios_fc::Message *message = mbox->createMessage();
    message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameAlive);
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
    if (evaluate())
        return;
    if (!message.hasInt(FPNetMessage::TYPE))
        return;
    int msgType = message.getInt(FPNetMessage::TYPE);
    if ((msgType == FPNetMessage::kGameSync)
        || (msgType == FPNetMessage::kGameAck)){
        if (!message.hasInt("SynID"))
            return;
        int synID = message.getInt("SynID");
        if (synID == m_synID) {
            if (msgType == FPNetMessage::kGameSync) {
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
    message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameSync);
    message->addInt     ("SynID", m_synID);
    message->send();
}

void NetSynchronizeState::sendAckMessage()
{
    auto_ptr<ios_fc::Message> message(m_mbox->createMessage());
    message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameAck);
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
        case NetworkGameWidget::NETWORK_FAILURE:
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
NetworkGameStateMachine::NetworkGameStateMachine(GameWidgetFactory  *gameWidgetFactory,
                                                 ios_fc::MessageBox *mbox,
                                                 GameDifficulty difficulty,
                                                 PlayerNameProvider *nameProvider,
                                                 Action *endOfSessionAction, int nbSets)
{
    // Creating the different game states
    m_pushGameScreen.reset(new PushScreenState());
    m_setupMatch.reset(new SetupMatchState(gameWidgetFactory, GameOptions::fromDifficulty(difficulty), nameProvider, &m_sharedAssets));
    m_enterPlayersReady.reset(new EnterPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_synchroGetReady.reset(new NetSynchronizeState(mbox, 1));
    m_exitPlayersReady.reset(new ExitPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    //m_waitPlayersReady.reset(new WaitPlayersReadyState(m_sharedAssets));
    m_synchroBeforeStart.reset(new NetSynchronizeState(mbox, 2));
    m_matchPlaying.reset(new NetMatchPlayingState(m_sharedAssets));
    m_matchIsOver.reset(new MatchIsOverState(m_sharedAssets));
    m_displayStats.reset(new DisplayStatsState(m_sharedAssets));
    m_podium.reset(new DisplayStoryScreenState("end_of_multiset.gsl"));
    m_synchroAfterStats.reset(new NetSynchronizeState(mbox, 10));
    m_manageMultiSets.reset(new ManageMultiSetsState(&m_sharedAssets, nbSets, nameProvider));
    m_networkErrorScreen.reset(new DisplayStoryScreenState("netfailure.gsl"));
    m_synchroOnAbort.reset(new NetSynchronizeState(mbox, 20));
    m_leaveGame.reset(new LeaveGameState(m_sharedAssets, endOfSessionAction));

    // Linking the states together
    m_pushGameScreen->setNextState(m_setupMatch.get());
    m_setupMatch->setNextState(m_enterPlayersReady.get());
    m_enterPlayersReady->setNextState(m_synchroGetReady.get());
    m_synchroGetReady->setNextState(m_exitPlayersReady.get());
    m_synchroGetReady->setFailedState(m_networkErrorScreen.get());
    m_exitPlayersReady->setNextState(m_synchroBeforeStart.get());
    m_synchroBeforeStart->setNextState(m_matchPlaying.get());
    m_synchroBeforeStart->setFailedState(m_networkErrorScreen.get());
    m_matchPlaying->setNextState(m_matchIsOver.get());
    m_matchPlaying->setAbortedState(m_synchroOnAbort.get());
    m_matchPlaying->setNetworkFailedState(m_networkErrorScreen.get());
    m_synchroOnAbort->setNextState(m_leaveGame.get());
    m_synchroOnAbort->setFailedState(m_networkErrorScreen.get());
    m_matchIsOver->setNextState(m_displayStats.get());
    m_displayStats->setNextState(m_synchroAfterStats.get());
    if (nbSets > 0) {
        m_synchroAfterStats->setNextState(m_manageMultiSets.get());
        m_manageMultiSets->setNextSetState(m_setupMatch.get());
        m_manageMultiSets->setEndOfGameState(m_podium.get());
        m_podium->setNextState(m_leaveGame.get());
        m_setupMatch->setHandicapOnVictorious(false);
        m_setupMatch->setDisplayVictories(true);
        m_podium->setStoryScreenValuesProvider(m_manageMultiSets.get());
    }
    else {
        m_synchroAfterStats->setNextState(m_setupMatch.get());
    }
    m_synchroAfterStats->setNextState(m_setupMatch.get());
    m_synchroAfterStats->setFailedState(m_networkErrorScreen.get());
    m_networkErrorScreen->setNextState(m_leaveGame.get());

    // Initializing the state machine
    setInitialState(m_pushGameScreen.get());
}

