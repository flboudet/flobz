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
#include "NetCenterMenu.h"
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
    void sendSyncMsg();
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
    bool syncMsgReceived, syncMsgSent;
    double lastMessageDate, lastAliveMessageSentDate;
    // Chat zone
    std::auto_ptr<ChatBox> chatBox; // optional
    // Network broken animation
    std::auto_ptr<StoryWidget> brokenNetworkWidget; // optional
    bool networkIsBroken;
};


#endif // _PUYONETWORKSTARTER
