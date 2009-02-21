#ifndef PUYO_SERVERV2_PEER_H
#define PUYO_SERVERV2_PEER_H

namespace flobopuyo {
namespace server {
namespace v2 {

class Peer {
public:
    Peer(ios_fc::PeerAddress addr, const ios_fc::String name)
      : addr(addr), name(name), status(0)
    {}
    ios_fc::PeerAddress addr;
    const ios_fc::String name;
    double lastUpdate;
    int status;
};

}}}

#endif
