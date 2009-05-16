#ifndef PUYO_SERVERV2_PEER_H
#define PUYO_SERVERV2_PEER_H

namespace flobopuyo {
namespace server {
namespace v2 {

class Peer {
public:
    Peer(ios_fc::PeerAddress addr, const ios_fc::String name, double timeMsBeforeTimeout)
      : addr(addr), name(name), status(0), mTimeMsBeforePeerTimeout(timeMsBeforeTimeout), rank(0)
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
    int rank;
    long int end_date;

private:
   double mTimeMsBeforePeerTimeout;
   double mLastUpdate;
};

}}}

#endif
