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
    NetworkGameFactory(RandomSystem *attachedRandom, MessageBox &msgBox, int gameId): attachedRandom(attachedRandom), msgBox(msgBox), gameId(gameId) {}
    FloboGame *createFloboGame(FloboFactory *attachedFloboFactory);
    int getGameId() { return gameId; }
private:
    RandomSystem *attachedRandom;
    MessageBox &msgBox;
    int gameId;
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
    virtual void sendChat(String chatText);
protected:
    void associatedScreenHasBeenSet(GameScreen *associatedScreen);
    virtual GamePlayer *createLocalPlayer();
private:
    void sendAliveMsg();
    FloboSetTheme *attachedFloboThemeSet; // optional
    std::unique_ptr<RandomSystem> attachedRandom;
    ios_fc::MessageBox *mbox;
    std::unique_ptr<LocalGameFactory> attachedLocalGameFactory;
    std::unique_ptr<NetworkGameFactory> attachedNetworkGameFactory;
protected:
    std::unique_ptr<NetworkGameView> localArea;
    std::unique_ptr<GameView> networkArea;
private:
    double lastMessageDate, lastAliveMessageSentDate;
    // Chat zone
    std::unique_ptr<ChatBox> chatBox; // optional
    // Network broken animation
    std::unique_ptr<StoryWidget> brokenNetworkWidget; // optional
    bool networkIsBroken;
    double m_networkTimeoutWarning, m_networkTimeoutError;
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
        m_nextState = nextState;
    }
    void setFailedState(GameState *failedState) {
        m_failedState = failedState;
    }
private:
    void sendSyncMessage();
    void sendAckMessage();
private:
    ios_fc::MessageBox *m_mbox;
    int m_synID;
    int m_cyclesTimeout;
    int m_cyclesCounter;
    bool m_synchronized, m_failed;
    bool m_ackSent;
    GameState *m_nextState, *m_failedState;
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
        m_netfailState = netfailState;
    }
private:
    bool m_networkFailure;
    GameState *m_netfailState;
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
    SharedMatchAssets           m_sharedAssets;
    SharedGetReadyAssets        m_sharedGetReadyAssets;
    std::unique_ptr<PushScreenState>         m_pushGameScreen;
    std::unique_ptr<SetupMatchState>         m_setupMatch;
    std::unique_ptr<EnterPlayerReadyState>   m_enterPlayersReady;
    std::unique_ptr<NetSynchronizeState>     m_synchroGetReady;
    std::unique_ptr<ExitPlayerReadyState>    m_exitPlayersReady;
    std::unique_ptr<NetSynchronizeState>     m_synchroBeforeStart;
    std::unique_ptr<NetMatchPlayingState>    m_matchPlaying;
    std::unique_ptr<MatchIsOverState>        m_matchIsOver;
    std::unique_ptr<DisplayStatsState>       m_displayStats;
    std::unique_ptr<ManageMultiSetsState>    m_manageMultiSets;
    std::unique_ptr<DisplayStoryScreenState> m_podium;
    std::unique_ptr<NetSynchronizeState>     m_synchroAfterStats;
    std::unique_ptr<DisplayStoryScreenState> m_networkErrorScreen;
    std::unique_ptr<NetSynchronizeState>     m_synchroOnAbort;
    std::unique_ptr<LeaveGameState>          m_leaveGame;
};

#endif // _NETWORKGAMESTARTER_H_
