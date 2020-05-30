#ifndef _IOS_HASH_H
#define _IOS_HASH_H

namespace ios_fc {

/**
 * Copyright (c) 2004, JC Hoelt.
 */

typedef struct IOS_HASH IosHash;

typedef union {
    void *ptr;
    char *str;
    long l;
    int i;
    float f;
} HashValue;

/* a hash map having a char* as key */
IosHash  *ios_hash_new       (unsigned int initial_size = 128);
void       ios_hash_free     (IosHash *gh);
HashValue *ios_hash_get      (IosHash *gh, const char *key);
void       ios_hash_put      (IosHash *gh, const char *key, HashValue value);
void       ios_hash_remove   (IosHash *gh, const char *key);

void ios_hash_put_int  (IosHash *_this,  const char *key, int i);
void ios_hash_put_float(IosHash *_this,  const char *key, float f);
void ios_hash_put_ptr  (IosHash *_this,  const char *key, void *ptr);



} // namespace ios_fc

#endif /* _IOS_HASH_H */
