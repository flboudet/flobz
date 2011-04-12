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
#include "PuyoTwoPlayerStarter.h"
#include "ios_messagebox.h"
#include "PuyoNetworkView.h"
#include "ChatBox.h"

class PuyoNetworkGameFactory : public PuyoGameFactory {
public:
    PuyoNetworkGameFactory(PuyoRandomSystem *attachedRandom, MessageBox &msgBox, int gameId): attachedRandom(attachedRandom), msgBox(msgBox), gameId(gameId) {}
    PuyoGame *createPuyoGame(PuyoFactory *attachedPuyoFactory);
    int getGameId() { return gameId; }
private:
    PuyoRandomSystem *attachedRandom;
    MessageBox &msgBox;
    int gameId;
};

class PuyoNetworkGameWidget : public GameWidget, MessageListener, ChatBoxDelegate {
public:
    enum {
        NETWORK_FAILURE = GAME_IS_OVER+1
    };
public:
    PuyoNetworkGameWidget();
    void initWithGUI(PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme, ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *gameOverAction = NULL, ios_fc::IgpMessageBox *igpbox = NULL);
    void initWithoutGUI(ios_fc::MessageBox &mbox, int gameId, unsigned long randomSeed, Action *gameOverAction = NULL, ios_fc::IgpMessageBox *igpbox = NULL);
    void connectIA(int level);
    ~PuyoNetworkGameWidget();
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
    virtual PuyoPlayer *createLocalPlayer();
private:
    void sendAliveMsg();
    PuyoSetTheme *attachedPuyoThemeSet; // optional
    std::auto_ptr<PuyoRandomSystem> attachedRandom;
    ios_fc::MessageBox *mbox;
    std::auto_ptr<PuyoLocalGameFactory> attachedLocalGameFactory;
    std::auto_ptr<PuyoNetworkGameFactory> attachedNetworkGameFactory;
protected:
    std::auto_ptr<PuyoNetworkView> localArea;
    std::auto_ptr<PuyoView> networkArea;
private:
    std::auto_ptr<PuyoPlayer> playercontroller;
    std::auto_ptr<PuyoNullPlayer> dummyPlayerController;
    double lastMessageDate, lastAliveMessageSentDate;
    // Chat zone
    std::auto_ptr<ChatBox> chatBox; // optional
    // Network broken animation
    std::auto_ptr<StoryWidget> brokenNetworkWidget; // optional
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
                            Action *endOfSessionAction = NULL);
private:
    SharedMatchAssets           m_sharedAssets;
    SharedGetReadyAssets        m_sharedGetReadyAssets;
    std::auto_ptr<PushScreenState>         m_pushGameScreen;
    std::auto_ptr<SetupMatchState>         m_setupMatch;
    std::auto_ptr<EnterPlayerReadyState>   m_enterPlayersReady;
    std::auto_ptr<NetSynchronizeState>     m_synchroGetReady;
    std::auto_ptr<ExitPlayerReadyState>    m_exitPlayersReady;
    std::auto_ptr<NetSynchronizeState>     m_synchroBeforeStart;
    std::auto_ptr<NetMatchPlayingState>    m_matchPlaying;
    std::auto_ptr<MatchIsOverState>        m_matchIsOver;
    std::auto_ptr<DisplayStatsState>       m_displayStats;
    std::auto_ptr<NetSynchronizeState>     m_synchroAfterStats;
    std::auto_ptr<DisplayStoryScreenState> m_networkErrorScreen;
    std::auto_ptr<NetSynchronizeState>     m_synchroOnAbort;
    std::auto_ptr<LeaveGameState>          m_leaveGame;
};

#endif // _PUYONETWORKSTARTER
