#ifndef PUYO_SERVERV2_PEER_H
#define PUYO_SERVERV2_PEER_H

namespace flobopuyo {
namespace server {
namespace v2 {

class Peer {
public:
    Peer(ios_fc::PeerAddress addr, const ios_fc::String name, double timeMsBeforeTimeout)
      : addr(addr), name(name), status(0), mTimeMsBeforePeerTimeout(timeMsBeforeTimeout)
    { touch(); }

    void touch() {
        mLastUpdate = ios_fc::getTimeMs();
    }
    bool isTimeout(double time_ms) {
        return (time_ms - mLastUpdate) > mTimeMsBeforePeerTimeout;
    }

    ios_fc::PeerAddress addr;
    const ios_fc::String name;
    int status;

private:
   double mTimeMsBeforePeerTimeout;
   double mLastUpdate;
};

}}}

#endif
