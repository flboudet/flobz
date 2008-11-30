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

class HashMapAction {
  public:
    virtual void action(HashValue *value) = 0;
    virtual ~HashMapAction() {};
};

// A Hashmap where keys are strings.
class HashMap {
    private:
        IosHash *hash;
    public:
        HashMap() : hash(ios_hash_new()) {}
        ~HashMap() { ios_hash_free(hash); }

        void put(const char *key, void *value) { ios_hash_put_ptr  (hash, key, value); }
        void put(const char *key, int   value) { ios_hash_put_int  (hash, key, value); }
        void put(const char *key, float value) { ios_hash_put_float(hash, key, value); }

        HashValue *get     (const char *key) const { return ios_hash_get(hash, key);      }
        /* HashValue *get_like(const char *key) const { return ios_hash_get_like(hash, key); } */
        void       remove  (const char *key)       { ios_hash_remove(hash, key);          }

        void foreach(HashMapAction *action);

    private:
        HashMap(const HashMap& hmap):hash(0){}
        HashMap& operator=(const HashMap& hmap)
        {
            hash=0;
            return *this;
        }
};

} // namespace ios_fc

#endif /* _IOS_HASH_H */
