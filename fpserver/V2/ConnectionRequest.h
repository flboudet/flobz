#ifndef CONNECTION_REQUEST_H
#define CONNECTION_REQUEST_H

namespace flobopuyo {
namespace server {
namespace v2 {

// Handles incoming connection requests
class ConnectionRequest
{
public:
    ConnectionRequest(const PeersList &peers, ios_fc::PeerAddress addr, int fpipVersion, const ios_fc::String name, int status)
    : mPeers(peers), mAddr(addr), mFpipVersion(fpipVersion), mName(name), mStatus(status)
    {}

    // Return true if the connection can be accepted
    bool isAcceptable() const;
    // Return the reason for denying connection
    const ios_fc::String &getDenyErrorString() const { return mDenyErrorString; }

private:
    const PeersList &mPeers;
    ios_fc::PeerAddress mAddr;
    int mFpipVersion;
    const ios_fc::String mName;
    int mStatus;
    mutable ios_fc::String mDenyErrorString;

    // Returns true if client protocol is supported
    bool checkVersion(int clientVersion) const;
};

bool ConnectionRequest::checkVersion(int clientVersion) const {
    return (clientVersion == 1);
}

bool ConnectionRequest::isAcceptable() const
{
    bool accept;
    
    // Check if the client FPIP version is compatible with the server
    if (!checkVersion(mFpipVersion)) {
        mDenyErrorString = "Client version mismatch.";
        return false;
    }
    // Check if we are exceeding the maximum number of allowed peers
    else if (mPeers.isFull()) {
        mDenyErrorString = "Too many connected players.";
        return false;
    }
    else {
        return true;
    }
}

}}}

#endif
