#include "PuyoInternetBot.h"
#include "PuyoStrings.h"
#include "preferences.h"
#include "PuyoNetworkView.h"

class PIBNGCListener : public PuyoNetGameCenterListener {
private:
    PuyoInternetGameCenter &m_gc;
    int m_level;
    MessageBox *m_opponent;
public:
    PIBNGCListener(PuyoInternetGameCenter &gc, int level) : m_gc(gc), m_level(level), m_opponent(NULL) {
    }

    virtual void onChatMessage(const String &msgAuthor, const String &msg) {
        std::cout << "onChatMessage" << std::endl;
    }

    virtual void onPlayerConnect(String playerName, PeerAddress playerAddress) {
        std::cout << "onPlayerConnect" << std::endl;
    }

    virtual void onPlayerDisconnect(String playerName, PeerAddress playerAddress) {
        std::cout << "onPlayerDisconnect" << std::endl;
    }

    virtual void onPlayerUpdated(String playerName, PeerAddress playerAddress) {
        std::cout << "onPlayerUpdated" << std::endl;
    }

    virtual void onGameInvitationReceived(PuyoGameInvitation &invitation) {
        std::cout << "onGameInvitationReceived" << std::endl;
        if (m_opponent == NULL)
            m_gc.acceptGameInvitation(invitation);
    }

    virtual void onGameInvitationCanceledReceived(PuyoGameInvitation &invitation) {
        std::cout << "onGameInvitationCanceledReceived" << std::endl;
    }

    std::auto_ptr<PuyoRandomSystem> m_myAttachedRandom;
    std::auto_ptr<PuyoLocalGameFactory> m_myAttachedPuyoGameFactory;
    std::auto_ptr<PuyoInternetNetworkView> m_myView;
    std::auto_ptr<PuyoIA> m_myAI;

    virtual void onGameGrantedWithMessagebox(MessageBox *mbox, PuyoGameInvitation &invitation) {
        std::cout << "onGameGrantedWithMessagebox" << std::endl;
        // My view
        m_opponent = mbox;
        m_myAttachedRandom = std::auto_ptr<PuyoRandomSystem>(new PuyoRandomSystem(5, invitation.gameRandomSeed));
        m_myAttachedPuyoGameFactory = std::auto_ptr<PuyoLocalGameFactory>(new PuyoLocalGameFactory(m_myAttachedRandom.get()));
        int gameId = 0;
        IgpMessageBox *igpbox = m_gc.getIgpBox();
        m_myView = std::auto_ptr<PuyoInternetNetworkView>(new PuyoInternetNetworkView(m_myAttachedPuyoGameFactory.get(), mbox, gameId, igpbox));
        m_myAI = std::auto_ptr<PuyoIA>(new PuyoIA(m_level, *m_myView));
        // Remote view
        // TODO
    }

    virtual void idle(double currentTime) {
    }
    
    virtual ~PIBNGCListener() {
    }
};

PuyoInternetBot::PuyoInternetBot(int level)
    : m_level(level)
{}

void PuyoInternetBot::connect(String server, int port, String name, String password)
{
    gameCenter = std::auto_ptr<PuyoInternetGameCenter>(new PuyoInternetGameCenter(server, port, name, password));
    gameCenterListener = std::auto_ptr<PIBNGCListener>(new PIBNGCListener(*gameCenter, m_level));
    gameCenter->addListener(gameCenterListener.get());
    std::cout << "Trying to connect..." << std::endl;
    while (!gameCenter->isConnected()) {
        gameCenter->idle();
        if (gameCenter->isDenied()) {
            std::cout << "Denied: " << gameCenter->getDenyString() << std::endl;
            gameCenter = std::auto_ptr<PuyoInternetGameCenter>(NULL);
            return;
        } 
    }
    std::cout << "Connected." << std::endl;
}

void PuyoInternetBot::idle(double currentTime)
{
  if (gameCenter.get() == NULL) return;
  gameCenter->idle();
  gameCenterListener->idle(currentTime);
}

