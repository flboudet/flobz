#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <signal.h>
#include "PuyoInternetGameCenter.h"

WINDOW *rootWin, *plyList, *chatArea, *inputArea;

class MyNetGameCenterListener : public PuyoNetGameCenterListener {
public:
    MyNetGameCenterListener(PuyoNetGameCenter &owner) : currentLine(0), owner(owner), chatLine(1) {}
    void onChatMessage(const String &msgAuthor, const String &msg) {
        wprintw(chatArea, "%s: %s\n", (const char *)msgAuthor, (const char *)msg);
        //if (chatLine > 10) wscrl(chatArea, 10);
        wrefresh(chatArea);
    }
    
    void drawPeers() {
        werase(plyList);
        wrefresh(plyList);
        box(plyList, ACS_VLINE, ACS_HLINE );
        wrefresh(plyList);
        for (int i = 0 ; i < owner.getPeerCount() ; i++) {
            mvwprintw(plyList, i+1, 1, "%s", (const char *)owner.getPeerNameAtIndex(i));
            wrefresh(plyList);
        }
        wrefresh(plyList);
        //mvwprintw(plyList, 1 + currentLine++, 1, "%s", (const char *)playerName);
    }
    
    void onPlayerConnect(String playerName, PeerAddress playerAddress) {
        drawPeers();
    }
    
    void onPlayerDisconnect(String playerName, PeerAddress playerAddress) {
        drawPeers();
    }
    
    void gameInvitationAgainst(String playerName, PeerAddress playerAddress) {
        wprintw(chatArea, "### %s wants to play against you\n", (const char *)playerName);
        wrefresh(chatArea);
    }
    void gameCanceledAgainst(String playerName, PeerAddress playerAddress) {
        wprintw(chatArea, "### The game against %s has been canceled\n", (const char *)playerName);
        wrefresh(chatArea);
    }
    void gameGrantedWithMessagebox(MessageBox *mbox) {}
private:
    int currentLine;
    PuyoNetGameCenter &owner;
    int chatLine;
};

static void finish(int sig)
{
    endwin();

    /* do your non-curses wrapup here */

    exit(0);
}

void decodeMessage(PuyoInternetGameCenter &myCenter, String msgBuf)
{
    if (msgBuf.substring(0, 6) == "/start") {
        String against = msgBuf.substring(7);
        wprintw(chatArea, "### Asking to start a game against %s\n", (const char *)against);
        wrefresh(chatArea);
        myCenter.requestGameWith(myCenter.getPeerAddressForPeerName(against));
    }
    else if (msgBuf.substring(0, 7) == "/cancel") {
        String against = msgBuf.substring(8);
        wprintw(chatArea, "### Asking to cancel a game against %s\n", (const char *)against);
        wrefresh(chatArea);
        myCenter.cancelGameWith(myCenter.getPeerAddressForPeerName(against));
    }
}

int main(int argc, char *argv[])
{
    int position = 0;
    bool mustUpdate = true;
    char msgBuf[256];
    signal(SIGINT, finish);

//#ifdef DISABL
    rootWin = initscr();      /* initialize the curses library */
    keypad(rootWin, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) noecho();       /* don't echo input */
    nodelay(rootWin, true);
    
    plyList = newwin(LINES - 6, 26, 1, 1);
    chatArea = newwin(LINES - 6, COLS - 28, 1, 27);
    scrollok(chatArea, TRUE);
    inputArea = newwin(4, COLS-2, LINES - 5, 1);
    box(rootWin, ACS_VLINE, ACS_HLINE );
    box(plyList, ACS_VLINE, ACS_HLINE );
    //box(chatArea, ACS_VLINE, ACS_HLINE );
    box(inputArea, ACS_VLINE, ACS_HLINE );
    refresh();
    wrefresh(chatArea);
    wrefresh(inputArea);
    wrefresh(plyList);
    
    PuyoInternetGameCenter myCenter(argv[1], 4567, argv[2]);
    MyNetGameCenterListener myListener(myCenter);
    myCenter.addListener(&myListener);
    while (1) {
        wmove(inputArea, 1, 1 + position);
        switch (int c = getch()) {
        case 0:
        case ERR:
            break;
        case 127:
        case KEY_BACKSPACE:
            wmove(inputArea, 1, position--);
            waddch(inputArea, ' ');
            //position--;
            break;
        case 13:
        case KEY_ENTER:
            msgBuf[position++] = 0;
            //wprintw(chatArea, "%s\n", (const char *)msgBuf);
            //wrefresh(chatArea);
            if ((position > 0) && (msgBuf[0] == '/'))
                decodeMessage(myCenter, msgBuf);
            else
                myCenter.sendMessage(msgBuf);
            werase(inputArea);
            box(inputArea, ACS_VLINE, ACS_HLINE );
            position = 0;
            break;
        default:
            //mvwprintw(plyList, 1, 1, "%d", c);
            msgBuf[position++] = c;
            waddch(inputArea, c);
            wrefresh(inputArea);
            break;
        }
        wrefresh(inputArea);
        myCenter.idle();
        usleep(20000);
    }
//#endif
    return 0;
}
