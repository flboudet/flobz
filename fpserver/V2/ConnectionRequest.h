#ifndef CONNECTION_REQUEST_H
#define CONNECTION_REQUEST_H

namespace flobopuyo {
namespace server {
namespace v2 {

// Handles incoming connection requests
class ConnectionRequest
{
public:
    ConnectionRequest(Database &db, const PeersList &peers, ios_fc::PeerAddress addr, int fpipVersion, const ios_fc::String name, const ios_fc::String password, int status)
    : mPeers(peers), mAddr(addr), mFpipVersion(fpipVersion), mName(name), mPassword(password), mStatus(status), mUserExists(false), mPasswordCorrect(false)
    {
        db.checkLogin(mName.c_str(), mPassword.c_str(), mUserExists,mPasswordCorrect);
    }

    // Return true if the connection can be accepted
    bool isAcceptable() const;
    // Return the reason for denying connection
    const ios_fc::String &getDenyErrorString() const { return mDenyErrorString; }

private:
    const PeersList &mPeers;
    ios_fc::PeerAddress mAddr;
    int mFpipVersion;
    const ios_fc::String mName;
    const ios_fc::String mPassword;
    int mStatus;
    mutable ios_fc::String mDenyErrorString;
    bool mUserExists;
    bool mPasswordCorrect;

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
        mDenyErrorString = "Err#01: Client version mismatch.";
        return false;
    }
    // Check if we are exceeding the maximum number of allowed peers
    else if (mPeers.isFull()) {
        mDenyErrorString = "Err#02: Too many connected players.";
        return false;
    }
    else if (!mUserExists) {
        mDenyErrorString = "Err#03: Incorrect user.";
        return false;
    }
    else if (!mPasswordCorrect) {
        mDenyErrorString = "Err#04: Incorrect password.";
        return false;
    }
    else {
        return true;
    }
}

}}}

#endif
