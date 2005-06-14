#ifndef _IOS_FC_H
#define _IOS_FC_H

#include "ios_memory.h"
//#include "ios_exception.h"
#include "ios_stream.h"
#include "ios_filestream.h"
#include "ios_hash.h"
#include "ios_message.h"
#include "ios_udpmessage.h"
#include "ios_vector.h"
#include "ios_filemanager.h"
#include "ios_time.h"

#include <stdlib.h>
#include <stdint.h>

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

#ifdef DEBUG
extern const char *DBG_PRINT_PREFIX;
#define DBG_PRINT_SET_PREFIX(txt) ios_fc::DBG_PRINT_PREFIX=strdup(txt);
#define DBG_PRINT(txt) fprintf(stderr, "%s: %s", ios_fc::DBG_PRINT_PREFIX, txt);
#else
#define DBG_PRINT(txt) (static_cast<void>(0))
#define DBG_PRINT_SET_PREFIX(txt) (static_cast<void>(0))
#endif
}

#endif
