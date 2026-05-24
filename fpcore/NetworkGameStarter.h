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
#ifndef _NETWORKGAMESTARTER_H_
#define _NETWORKGAMESTARTER_H_

#include "StoryModeStarter.h"
#include "TwoPlayersGameStarter.h"
#include "ios_messagebox.h"
#include "NetworkGameView.h"
#include "ChatBox.h"

class NetworkGameFactory : public FloboGameFactory {
public:
    NetworkGameFactory(RandomSystem *attachedRandom, MessageBox &msgBox, int gameId): _attachedRandom(attachedRandom), _msgBox(msgBox), _gameId(gameId) {}
    FloboGame *createFloboGame(FloboFactory *attachedFloboFactory);
    int getGameId() { return _gameId; }
private:
    RandomSystem *_attachedRandom;
    MessageBox &_msgBox;
    int _gameId;
};

class NetworkGameWidget : public GameWidget2P, MessageListener, ChatBoxDelegate {
public:
    enum {
        NETWORK_FAILURE = GAME_IS_OVER+1
    };
public:
    NetworkGameWidget();
    void initWithGUI(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *gameOverAction = NULL, FPServerIGPMessageBox *igpbox = NULL, bool hasChatBox = true);
    void initWithoutGUI(ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *gameOverAction = NULL, FPServerIGPMessageBox *igpbox = NULL);
    void connectIA(int level);
    ~NetworkGameWidget();
    bool didPlayerWon() const { return isGameARunning(); }
    void cycle();
    void onMessage(Message &);
    void setScreenToPaused(bool fromControls);
    void setScreenToResumed(bool fromControls);
    void abort();
    void actionAfterGameOver(bool fromControls, int actionType);
    virtual void sendChat(const std::string &chatText);
protected:
    void associatedScreenHasBeenSet(GameScreen *associatedScreen);
    virtual GamePlayer *createLocalPlayer();
private:
    void sendAliveMsg();
    FloboSetTheme *_attachedFloboThemeSet; // optional
    std::unique_ptr<RandomSystem> _attachedRandom;
    ios_fc::MessageBox *_mbox;
    std::unique_ptr<LocalGameFactory> _attachedLocalGameFactory;
    std::unique_ptr<NetworkGameFactory> _attachedNetworkGameFactory;
protected:
    std::unique_ptr<NetworkGameView> _localArea;
    std::unique_ptr<GameView> _networkArea;
private:
    double _lastMessageDate, _lastAliveMessageSentDate;
    // Chat zone
    std::unique_ptr<ChatBox> _chatBox; // optional
    // Network broken animation
    std::unique_ptr<StoryWidget> _brokenNetworkWidget; // optional
    bool _networkIsBroken;
    double _networkTimeoutWarning, _networkTimeoutError;
};

/**
 * Wait for peers of a network game to synchronize
 */
class NetSynchronizeState : public GameState, MessageListener, CycledComponent
{
public:
    NetSynchronizeState(ios_fc::MessageBox *mbox,
                        int synID, double timeoutSec = 10.);
    // GameState implementation
    virtual void enterState();
    virtual void exitState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // MessageListener implementation
    void onMessage(Message &);
    // CycledComponent implementation
    virtual void cycle();
    // Own methods
    void setNextState(GameState *nextState) {
        _nextState = nextState;
    }
    void setFailedState(GameState *failedState) {
        _failedState = failedState;
    }
private:
    void sendSyncMessage();
    void sendAckMessage();
private:
    ios_fc::MessageBox *_mbox;
    int _synID;
    int _cyclesTimeout;
    int _cyclesCounter;
    bool _synchronized, _failed;
    bool _ackSent;
    GameState *_nextState, *_failedState;
};

/**
 * Make the match play until it is finished or aborted
 * Slightly overloaded network version to handle network errors
 */
class NetMatchPlayingState : public MatchPlayingState
{
public:
    NetMatchPlayingState(SharedMatchAssets &sharedMatchAssets);
    // GameState implementation
    virtual void enterState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // Action implementation
    virtual void action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event);
    void setNetworkFailedState(GameState *netfailState) {
        _netfailState = netfailState;
    }
private:
    bool _networkFailure;
    GameState *_netfailState;
};

class NetworkGameStateMachine : public GameStateMachine
{
public:
    NetworkGameStateMachine(GameWidgetFactory  *gameWidgetFactory,
                            ios_fc::MessageBox *mbox,
                            GameDifficulty difficulty,
                            PlayerNameProvider *nameProvider = NULL,
                            Action *endOfSessionAction = NULL,
                            int nbSets = 0);
private:
    SharedMatchAssets           _sharedAssets;
    SharedGetReadyAssets        _sharedGetReadyAssets;
    std::unique_ptr<PushScreenState>         _pushGameScreen;
    std::unique_ptr<SetupMatchState>         _setupMatch;
    std::unique_ptr<EnterPlayerReadyState>   _enterPlayersReady;
    std::unique_ptr<NetSynchronizeState>     _synchroGetReady;
    std::unique_ptr<ExitPlayerReadyState>    _exitPlayersReady;
    std::unique_ptr<NetSynchronizeState>     _synchroBeforeStart;
    std::unique_ptr<NetMatchPlayingState>    _matchPlaying;
    std::unique_ptr<MatchIsOverState>        _matchIsOver;
    std::unique_ptr<DisplayStatsState>       _displayStats;
    std::unique_ptr<ManageMultiSetsState>    _manageMultiSets;
    std::unique_ptr<DisplayStoryScreenState> _podium;
    std::unique_ptr<NetSynchronizeState>     _synchroAfterStats;
    std::unique_ptr<DisplayStoryScreenState> _networkErrorScreen;
    std::unique_ptr<NetSynchronizeState>     _synchroOnAbort;
    std::unique_ptr<LeaveGameState>          _leaveGame;
};

#endif // _NETWORKGAMESTARTER_H_
