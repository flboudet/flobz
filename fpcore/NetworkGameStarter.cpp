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
    return new NetworkGame(attachedFloboFactory, &_msgBox, _gameId);
}

NetworkGameWidget::NetworkGameWidget()
    : _chatBox(),
      _brokenNetworkWidget(), _networkIsBroken(false),
      _networkTimeoutWarning(5000.),
      _networkTimeoutError(15000.)
{
}

GamePlayer *NetworkGameWidget::createLocalPlayer()
{
    return new CombinedEventPlayer(*_localArea);
}

void NetworkGameWidget::initWithGUI(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *_gameOverAction, FPServerIGPMessageBox *igpbox, bool hasChatBox)
{
    _attachedFloboThemeSet = &floboSetTheme;
    _attachedRandom = std::unique_ptr<RandomSystem>(new RandomSystem(randomSeed, 5));
    this->_mbox = &mbox;
    _attachedLocalGameFactory   = std::unique_ptr<LocalGameFactory>(new LocalGameFactory(_attachedRandom.get()));
    _attachedNetworkGameFactory = std::unique_ptr<NetworkGameFactory>(new NetworkGameFactory(_attachedRandom.get(), *_mbox, gameId));
    if (igpbox != NULL) {
        _localArea = std::unique_ptr<NetworkGameView>(new InternetGameView(_attachedLocalGameFactory.get(), 0, _attachedFloboThemeSet, &levelTheme,
                                            &mbox, gameId, igpbox));
    }
    else {
        _localArea = std::unique_ptr<NetworkGameView>(new NetworkGameView(_attachedLocalGameFactory.get(), 0, _attachedFloboThemeSet, &levelTheme, &mbox, gameId));
    }
    _networkArea = std::unique_ptr<GameView>(new GameView(_attachedNetworkGameFactory.get(), 1, _attachedFloboThemeSet, &levelTheme));
    this->_mbox->addListener(this);
    if (hasChatBox)
        _chatBox = std::unique_ptr<ChatBox>(new ChatBox(*this));
    _brokenNetworkWidget = std::unique_ptr<StoryWidget>(new StoryWidget("etherdown.gsl"));
    _networkIsBroken = false;
    GameWidget2P::initWithGUI(*_localArea, *_networkArea, levelTheme, _gameOverAction);
    _controllerA.reset(createLocalPlayer());
    _controllerB.reset(new GameNullPlayer(*_networkArea));
    setLives(-1);
}

void NetworkGameWidget::initWithoutGUI(ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *_gameOverAction, FPServerIGPMessageBox *igpbox)
{
    this->_mbox->addListener(this);
    //GameWidget::initWithoutGUI(*localArea, networkArea, playercontroller, dummyPlayerController, levelTheme, _gameOverAction);
    setLives(-1);
}

void NetworkGameWidget::connectIA(int level)
{
}

NetworkGameWidget::~NetworkGameWidget()
{
    _mbox->removeListener(this);
    // delete(localArea);
}

void NetworkGameWidget::cycle()
{
    double curDate = ios_fc::getTimeMs();
    if (_paused) {
        if (curDate - _lastAliveMessageSentDate > 2000.) {
            sendAliveMsg();
            _lastAliveMessageSentDate = curDate;
        }
    }
    else {
        // TODO: Check player's height
        theCommander->playMusicTrack("level1");
    }
    // Management of network timeouts
    // (what to do when the network stops responding?)
    if (curDate - _lastMessageDate > _networkTimeoutWarning) {
        if (!_networkIsBroken) {
            _associatedScreen->add(_brokenNetworkWidget.get());
            _networkIsBroken = true;
        }
        else if (curDate - _lastMessageDate > _networkTimeoutError) {
            // Call network failure action
            if (_gameOverAction)
                _gameOverAction->action(this, NETWORK_FAILURE, NULL);
        }
    }
    else if (_networkIsBroken == true) {
        _associatedScreen->remove(_brokenNetworkWidget.get());
        _networkIsBroken = false;
    }
    // Let the game behave
    GameWidget2P::cycle();
    _mbox->idle();
}

void NetworkGameWidget::onMessage(Message &message)
{
    if (message.hasInt(FPNetMessage::GAMEID)) {
        _lastMessageDate = ios_fc::getTimeMs();
    }
    if (!message.hasInt(FPNetMessage::TYPE))
        return;
    _lastMessageDate = ios_fc::getTimeMs();
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
            if (_chatBox.get())
                _chatBox->addChat(message.getString("NAME").c_str(), message.getString("TEXT").c_str()); // TODO: string
            printf("%s: %s\n", message.getString("NAME").c_str(), message.getString("TEXT").c_str());
            break;
        default:
            break;
    }
}

void NetworkGameWidget::setScreenToPaused(bool fromControls)
{
    // If the pause is from a controller, we have to send the pause information to the other peer
    if (fromControls) {
        ios_fc::Message *message = _mbox->createMessage();
        message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGamePause);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    _lastAliveMessageSentDate = ios_fc::getTimeMs();
    GameWidget2P::setScreenToPaused(fromControls);
}

void NetworkGameWidget::setScreenToResumed(bool fromControls)
{
    // If the resume is from a controller, we have to send the resume information to the other peer
    if (fromControls) {
        ios_fc::Message *message = _mbox->createMessage();
        message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameResume);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    GameWidget2P::setScreenToResumed(fromControls);
}

void NetworkGameWidget::abort()
{
    ios_fc::Message *message = _mbox->createMessage();
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
        ios_fc::Message *message = _mbox->createMessage();
        message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameNext);
        message->addBoolProperty("RELIABLE", true);
        message->send();
        delete message;
    }
    GameWidget2P::actionAfterGameOver(fromControls, actionType);
}

void NetworkGameWidget::sendChat(const std::string &chatText)
{
    ios_fc::Message *message = _mbox->createMessage();
    message->addInt(FPNetMessage::TYPE,   FPNetMessage::kGameChat);
    message->addString("NAME",   getPlayerOneName());
    message->addString("TEXT",   chatText);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    if (_chatBox.get())
        _chatBox->addChat(getPlayerOneName(), chatText);
    delete message;
}

void NetworkGameWidget::associatedScreenHasBeenSet(GameScreen *associatedScreen)
{
    if (_chatBox.get()) {
        _associatedScreen->getPauseMenu().add(_chatBox.get());
        _associatedScreen->getPauseMenu()._pauseMenuTop = 5;
    }
}

void NetworkGameWidget::sendAliveMsg()
{
    ios_fc::Message *message = _mbox->createMessage();
    message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameAlive);
    message->send();
    delete message;
}

//---------------------------------
// NetSynchronizeState
//---------------------------------
NetSynchronizeState::NetSynchronizeState(ios_fc::MessageBox *mbox,
                                         int synID, double timeoutSec)
    : CycledComponent(0.1), _mbox(mbox), _synID(synID), _cyclesTimeout(timeoutSec/0.1)
{
}

void NetSynchronizeState::enterState()
{
    cout << "NetSynchronizeState("<<_synID<<")::enterState()" << endl;
    GameUIDefaults::GAME_LOOP->addIdle(this);
    _synchronized = false;
    _failed = false;
    _ackSent = false;
    _cyclesCounter = 0;
    _mbox->addListener(this);
    // Send synchro message
    sendSyncMessage();
}

void NetSynchronizeState::exitState()
{
    GameUIDefaults::GAME_LOOP->removeIdle(this);
    _mbox->removeListener(this);
    _failed = false;
    _ackSent = false;
}

bool NetSynchronizeState::evaluate()
{
    return ((_synchronized && _ackSent) || (_failed));
}

GameState *NetSynchronizeState::getNextState()
{
    if (_failed)
        return _failedState;
    return _nextState;
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
        if (synID == _synID) {
            if (msgType == FPNetMessage::kGameSync) {
                if (!_ackSent) {
                    sendAckMessage();
                    _ackSent = true;
                }
                evaluateStateMachine();
            }
            else {
                _synchronized = true;
                evaluateStateMachine();
            }
        }
    }
}

void NetSynchronizeState::cycle()
{
    if (_cyclesCounter++ == _cyclesTimeout) {
        _failed = true;
        evaluateStateMachine();
    }
    sendSyncMessage();
}

void NetSynchronizeState::sendSyncMessage()
{
    unique_ptr<ios_fc::Message> message(_mbox->createMessage());
    message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameSync);
    message->addInt     ("SynID", _synID);
    message->send();
}

void NetSynchronizeState::sendAckMessage()
{
    unique_ptr<ios_fc::Message> message(_mbox->createMessage());
    message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameAck);
    message->addInt     ("SynID", _synID);
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
    _networkFailure = false;
    MatchPlayingState::enterState();
}

bool NetMatchPlayingState::evaluate()
{
    if (_networkFailure)
        return true;
    return MatchPlayingState::evaluate();
}

GameState *NetMatchPlayingState::getNextState()
{
    if (_networkFailure)
        return _netfailState;
    return MatchPlayingState::getNextState();
}

void NetMatchPlayingState::action(Widget *sender, int actionType,
                                  event_manager::GameControlEvent *event)
{
    switch (actionType) {
        case NetworkGameWidget::NETWORK_FAILURE:
            _networkFailure = true;
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
    _pushGameScreen.reset(new PushScreenState());
    _setupMatch.reset(new SetupMatchState(gameWidgetFactory, GameOptions::fromDifficulty(difficulty), nameProvider, &_sharedAssets));
    _enterPlayersReady.reset(new EnterPlayerReadyState(_sharedAssets, _sharedGetReadyAssets));
    _synchroGetReady.reset(new NetSynchronizeState(mbox, 1));
    _exitPlayersReady.reset(new ExitPlayerReadyState(_sharedAssets, _sharedGetReadyAssets));
    //_waitPlayersReady.reset(new WaitPlayersReadyState(_sharedAssets));
    _synchroBeforeStart.reset(new NetSynchronizeState(mbox, 2));
    _matchPlaying.reset(new NetMatchPlayingState(_sharedAssets));
    _matchIsOver.reset(new MatchIsOverState(_sharedAssets));
    _displayStats.reset(new DisplayStatsState(_sharedAssets));
    _podium.reset(new DisplayStoryScreenState("end_of_multiset.gsl"));
    _synchroAfterStats.reset(new NetSynchronizeState(mbox, 10));
    _manageMultiSets.reset(new ManageMultiSetsState(&_sharedAssets, nbSets, nameProvider));
    _networkErrorScreen.reset(new DisplayStoryScreenState("netfailure.gsl"));
    _synchroOnAbort.reset(new NetSynchronizeState(mbox, 20));
    _leaveGame.reset(new LeaveGameState(_sharedAssets, endOfSessionAction));

    // Linking the states together
    _pushGameScreen->setNextState(_setupMatch.get());
    _setupMatch->setNextState(_enterPlayersReady.get());
    _enterPlayersReady->setNextState(_synchroGetReady.get());
    _synchroGetReady->setNextState(_exitPlayersReady.get());
    _synchroGetReady->setFailedState(_networkErrorScreen.get());
    _exitPlayersReady->setNextState(_synchroBeforeStart.get());
    _synchroBeforeStart->setNextState(_matchPlaying.get());
    _synchroBeforeStart->setFailedState(_networkErrorScreen.get());
    _matchPlaying->setNextState(_matchIsOver.get());
    _matchPlaying->setAbortedState(_synchroOnAbort.get());
    _matchPlaying->setNetworkFailedState(_networkErrorScreen.get());
    _synchroOnAbort->setNextState(_leaveGame.get());
    _synchroOnAbort->setFailedState(_networkErrorScreen.get());
    _matchIsOver->setNextState(_displayStats.get());
    _displayStats->setNextState(_synchroAfterStats.get());
    if (nbSets > 0) {
        _synchroAfterStats->setNextState(_manageMultiSets.get());
        _manageMultiSets->setNextSetState(_setupMatch.get());
        _manageMultiSets->setEndOfGameState(_podium.get());
        _podium->setNextState(_leaveGame.get());
        _setupMatch->setHandicapOnVictorious(false);
        _setupMatch->setDisplayVictories(true);
        _podium->setStoryScreenValuesProvider(_manageMultiSets.get());
    }
    else {
        _synchroAfterStats->setNextState(_setupMatch.get());
    }
    _synchroAfterStats->setFailedState(_networkErrorScreen.get());
    _networkErrorScreen->setNextState(_leaveGame.get());

    // Initializing the state machine
    setInitialState(_pushGameScreen.get());
}
