#ifndef _PUYOPEERSLISTENER_H
#define _PUYOPEERSLISTENER_H

#include "ios_igpvirtualpeermessagebox.h"
#include "ios_memory.h"
#include "ios_dirigeable.h"

namespace flobopuyo {
namespace server {

class PuyoServer : public ios_fc::MessageListener {
public:
    virtual void idle() = 0;
    virtual ~PuyoServer() {}
};

}}

#endif

