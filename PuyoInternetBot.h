#ifndef PUYOINTERNETBOT_H
#define PUYOINTERNETBOT_H

#include "gametools/gameui.h"
#include "MainMenu.h"
#include <memory>

class PIBNGCListener;

class PuyoInternetBot : public IdleComponent
{
  public:
    PuyoInternetBot(int level);

    void connect(String server, int port, String name, String password);
    void idle(double currentTime);

  private:
    int m_level;
    std::auto_ptr<PuyoInternetGameCenter> gameCenter;
    std::auto_ptr<PIBNGCListener> gameCenterListener;
};

#endif
