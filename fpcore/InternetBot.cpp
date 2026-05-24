#include "InternetBot.h"
#include "FPStrings.h"
#include "NetworkGameView.h"
#include "NetworkGameStarter.h"

class PIBNGCListener : public NetGameCenterListener {
private:
    InternetGameCenter &_gc;
    int _level;
    MessageBox *_opponent;
public:
    PIBNGCListener(InternetGameCenter &gc, int level) : _gc(gc), _level(level), _opponent(NULL) {
    }

    virtual void onChatMessage(const std::string &msgAuthor, const std::string &msg) {
        std::cout << "onChatMessage" << std::endl;
    }

    virtual void onPlayerConnect(const std::string & playerName, PeerAddress playerAddress) {
        std::cout << "onPlayerConnect" << std::endl;
    }

    virtual void onPlayerDisconnect(const std::string & playerName, PeerAddress playerAddress) {
        std::cout << "onPlayerDisconnect" << std::endl;
    }

    virtual void onPlayerUpdated(const std::string & playerName, PeerAddress playerAddress) {
        std::cout << "onPlayerUpdated" << std::endl;
    }

    virtual void onGameInvitationReceived(FloboGameInvitation &invitation) {
        std::cout << "onGameInvitationReceived" << std::endl;
        if (_opponent == NULL)
            _gc.acceptGameInvitation(invitation);
    }

    virtual void onGameInvitationCanceledReceived(FloboGameInvitation &invitation) {
        std::cout << "onGameInvitationCanceledReceived" << std::endl;
    }

    /*
    std::unique_ptr<RandomSystem> _myRandom;
    std::unique_ptr<LocalGameFactory> _myGameFactory;
    std::unique_ptr<InternetGameView> _myView;
    std::unique_ptr<AIPlayer> _myAI;

    std::unique_ptr<RandomSystem> _opRandom;
    std::unique_ptr<NetworkGameFactory> _opGameFactory;
    std::unique_ptr<GameView> _opView;
    */
    std::unique_ptr<NetworkGameWidget> _Negawi; // the NEtwork GAme WIdget

    virtual void onGameGrantedWithMessagebox(MessageBox *mbox, FloboGameInvitation &invitation) {
        std::cout << "onGameGrantedWithMessagebox" << std::endl;
        int gameId = 0;
        FPServerIGPMessageBox *igpbox = _gc.getIgpBox();
        /*
        // My view
        _opponent = mbox;
        _myRandom = std::unique_ptr<RandomSystem>(new RandomSystem(5, invitation._gameRandomSeed));
        _myGameFactory = std::unique_ptr<LocalGameFactory>(new LocalGameFactory(_myRandom.get()));
        _myView = std::unique_ptr<InternetGameView>(new InternetGameView(_myGameFactory.get(), mbox, gameId, igpbox));
        _myAI = std::unique_ptr<AIPlayer>(new AIPlayer(_level, *_myView));
        // Remote view
        // TODO
        _opRandom = std::unique_ptr<RandomSystem>(new RandomSystem(5, invitation._gameRandomSeed));
        _opGameFactory = std::unique_ptr<NetworkGameFactory>(new NetworkGameFactory(_opRandom.get(), *mbox, gameId));
        _opView = std::unique_ptr<GameView>(new GameView(_opGameFactory.get()));
        */
        _Negawi = std::unique_ptr<NetworkGameWidget>(new NetworkGameWidget());
        _Negawi->initWithoutGUI(*mbox, gameId, invitation._gameRandomSeed, NULL, igpbox);
        _Negawi->connectIA(_level);
    }

    virtual void idle(double currentTime) {
        // Nothing happens...
    }

    virtual ~PIBNGCListener() {
    }
};

InternetBot::InternetBot(int level)
    : _level(level)
{}

void InternetBot::connect(const std::string & server, int port, const std::string & name, const std::string & password)
{
    _gameCenter = std::unique_ptr<InternetGameCenter>(new InternetGameCenter(server, port, name, password));
    _gameCenterListener = std::unique_ptr<PIBNGCListener>(new PIBNGCListener(*_gameCenter, _level));
    _gameCenter->addListener(_gameCenterListener.get());
    std::cout << "Trying to connect..." << std::endl;
    while (!_gameCenter->isConnected()) {
        _gameCenter->idle();
        if (_gameCenter->isDenied()) {
            std::cout << "Denied: " << _gameCenter->getDenyString() << std::endl;
            _gameCenter = std::unique_ptr<InternetGameCenter>();
            return;
        }
    }
    std::cout << "Connected." << std::endl;
}

void InternetBot::idle(double currentTime)
{
  if (_gameCenter.get() == NULL) return;
  _gameCenter->idle();
  _gameCenterListener->idle(currentTime);
}

