#ifndef _IOS_FC
#define _IOS_FC

#include "ios_memory.h"
// #include "ios_exception.h"
#include "ios_stream.h"
#include "ios_filestream.h"
#include "ios_hash.h"
#include "ios_message.h"
#include "ios_udpmessage.h"
#include "ios_vector.h"

#include <stdlib.h>

namespace ios_fc {

    inline int irand(int i) {
        return (::rand() / 255) % i;
    }

    inline float frand() {
        return ((float)::rand() / RAND_MAX);
    }
    
    inline float frand(float f) {
        return f * ((float)::rand() / RAND_MAX);
    }
};

#endif
