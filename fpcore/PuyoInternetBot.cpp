#include "PuyoInternetBot.h"
#include "PuyoStrings.h"
#include "preferences.h"
#include "PuyoNetworkView.h"
#include "PuyoNetworkStarter.h"

class PIBNGCListener : public NetGameCenterListener {
private:
    InternetGameCenter &m_gc;
    int m_level;
    MessageBox *m_opponent;
public:
    PIBNGCListener(InternetGameCenter &gc, int level) : m_gc(gc), m_level(level), m_opponent(NULL) {
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

    /*
    std::auto_ptr<PuyoRandomSystem> m_myRandom;
    std::auto_ptr<PuyoLocalGameFactory> m_myGameFactory;
    std::auto_ptr<PuyoInternetNetworkView> m_myView;
    std::auto_ptr<AIPlayer> m_myAI;

    std::auto_ptr<PuyoRandomSystem> m_opRandom;
    std::auto_ptr<PuyoNetworkGameFactory> m_opGameFactory;
    std::auto_ptr<PuyoView> m_opView;
    */
    std::auto_ptr<PuyoNetworkGameWidget> m_Negawi; // the NEtwork GAme WIdget

    virtual void onGameGrantedWithMessagebox(MessageBox *mbox, PuyoGameInvitation &invitation) {
        std::cout << "onGameGrantedWithMessagebox" << std::endl;
        int gameId = 0;
        IgpMessageBox *igpbox = m_gc.getIgpBox();
        /*
        // My view
        m_opponent = mbox;
        m_myRandom = std::auto_ptr<PuyoRandomSystem>(new PuyoRandomSystem(5, invitation.gameRandomSeed));
        m_myGameFactory = std::auto_ptr<PuyoLocalGameFactory>(new PuyoLocalGameFactory(m_myRandom.get()));
        m_myView = std::auto_ptr<PuyoInternetNetworkView>(new PuyoInternetNetworkView(m_myGameFactory.get(), mbox, gameId, igpbox));
        m_myAI = std::auto_ptr<AIPlayer>(new AIPlayer(m_level, *m_myView));
        // Remote view
        // TODO
        m_opRandom = std::auto_ptr<PuyoRandomSystem>(new PuyoRandomSystem(5, invitation.gameRandomSeed));
        m_opGameFactory = std::auto_ptr<PuyoNetworkGameFactory>(new PuyoNetworkGameFactory(m_opRandom.get(), *mbox, gameId));
        m_opView = std::auto_ptr<PuyoView>(new PuyoView(m_opGameFactory.get()));
        */
        m_Negawi = std::auto_ptr<PuyoNetworkGameWidget>(new PuyoNetworkGameWidget());
        m_Negawi->initWithoutGUI(*mbox, gameId, invitation.gameRandomSeed, NULL, igpbox);
        m_Negawi->connectIA(m_level);
    }

    virtual void idle(double currentTime) {
        // Nothing happens...
    }
    
    virtual ~PIBNGCListener() {
    }
};

PuyoInternetBot::PuyoInternetBot(int level)
    : m_level(level)
{}

void PuyoInternetBot::connect(String server, int port, String name, String password)
{
    gameCenter = std::auto_ptr<InternetGameCenter>(new InternetGameCenter(server, port, name, password));
    gameCenterListener = std::auto_ptr<PIBNGCListener>(new PIBNGCListener(*gameCenter, m_level));
    gameCenter->addListener(gameCenterListener.get());
    std::cout << "Trying to connect..." << std::endl;
    while (!gameCenter->isConnected()) {
        gameCenter->idle();
        if (gameCenter->isDenied()) {
            std::cout << "Denied: " << gameCenter->getDenyString() << std::endl;
            gameCenter = std::auto_ptr<InternetGameCenter>(NULL);
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

