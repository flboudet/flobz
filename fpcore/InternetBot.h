#ifndef PUYOINTERNETBOT_H
#define PUYOINTERNETBOT_H

#include "gameui.h"
#include "MainMenu.h"
#include <memory>

class PIBNGCListener;

class InternetBot : public IdleComponent
{
  public:
    InternetBot(int level);

    void connect(String server, int port, String name, String password);
    void idle(double currentTime);

  private:
    int m_level;
    std::unique_ptr<InternetGameCenter> gameCenter;
    std::unique_ptr<PIBNGCListener> gameCenterListener;
};

#endif
