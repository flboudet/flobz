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

    void connect(const std::string & server, int port, const std::string & name, const std::string & password);
    void idle(double currentTime);

  private:
    int _level;
    std::unique_ptr<InternetGameCenter> _gameCenter;
    std::unique_ptr<PIBNGCListener> _gameCenterListener;
};

#endif
