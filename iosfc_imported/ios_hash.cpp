#include "ios_hash.h"

#include "google/dense_hash_map" // Time efficient hashmap
using google::dense_hash_map;

#include <string.h>
#include <stdint.h>

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
      || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                               +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

namespace ios_fc {

#define EMPTY_KEY ((const char*)1)
#define DELETED_KEY (NULL)

// by Paul Hsieh
// http://www.azillionmonkeys.com/qed/hash.html
struct SuperFastHashString {
    size_t operator()(const char *data) const {
        if ((data == EMPTY_KEY) || (data == DELETED_KEY)) return 0;
        uint32_t len = strlen(data);
        uint32_t hash = len, tmp;
        int rem;

        if (len <= 0 || data == NULL) return 0;

        rem = len & 3;
        len >>= 2;

        /* Main loop */
        for (;len > 0; len--) {
            hash  += get16bits (data);
            tmp    = (get16bits (data+2) << 11) ^ hash;
            hash   = (hash << 16) ^ tmp;
            data  += 2*sizeof (uint16_t);
            hash  += hash >> 11;
        }

        /* Handle end cases */
        switch (rem) {
            case 3: hash += get16bits (data);
                    hash ^= hash << 16;
                    hash ^= data[sizeof (uint16_t)] << 18;
                    hash += hash >> 11;
                    break;
            case 2: hash += get16bits (data);
                    hash ^= hash << 11;
                    hash += hash >> 17;
                    break;
            case 1: hash += *data;
                    hash ^= hash << 10;
                    hash += hash >> 1;
        }

        /* Force "avalanching" of final 127 bits */
        hash ^= hash << 3;
        hash += hash >> 5;
        hash ^= hash << 4;
        hash += hash >> 17;
        hash ^= hash << 25;
        hash += hash >> 6;

        return hash;
    }
};

struct EqualString {
    bool operator()(const char* s1, const char* s2) const {
        return (s1 == s2) || (
                (s1 != EMPTY_KEY) &&
                (s2 != DELETED_KEY) &&
                (s1 != DELETED_KEY) &&
                (s2 != EMPTY_KEY) && strcmp(s1, s2) == 0);
    }
};
typedef dense_hash_map<const char*, HashValue, SuperFastHashString, EqualString> gg_str_hashmap;

struct IOS_HASH {
    gg_str_hashmap root;
};

/**
 * Copyright (c) 2004 JC Hoelt.
 */

IosHash *ios_hash_new(unsigned int initial_size) {
	IosHash *_this = new IOS_HASH;
    _this->root.set_empty_key((const char*)1);
    _this->root.set_deleted_key(NULL);
	return _this;
}

void ios_hash_free(IosHash *_this) {
    gg_str_hashmap::iterator it = _this->root.begin();
    while (it != _this->root.end()) {
        free((void*)(*it).first);
        ++ it;
    }
	delete _this;
}

void ios_hash_put(IosHash *_this, const char *key, HashValue value) {
    if (_this == NULL) return;
    int len    = strlen(key);
	char *key2 = (char*)malloc(len+1);
	memcpy(key2,key,len+1);
    _this->root[key2] = value;
}

HashValue *ios_hash_get(IosHash *_this, const char *key) {
    if (_this == NULL) return NULL;
    gg_str_hashmap::iterator it = _this->root.find(key);
    if (it == _this->root.end())
        return NULL;
    else if ((*it).first == DELETED_KEY)
        return NULL;
    else if ((*it).first == EMPTY_KEY)
        return NULL;
    else
        return &((*it).second);
}

void ios_hash_remove (IosHash *_this, const char *key) {
    gg_str_hashmap::iterator it = _this->root.find(key);
    if ((it != _this->root.end()) && ((*it).first != DELETED_KEY) && ((*it).first != EMPTY_KEY)) {
        free((void*)(*it).first);
        _this->root.erase(it);
    }
}

void ios_hash_put_int(IosHash *_this, const char *key, int i) {
    HashValue value;
    value.i = i;
    ios_hash_put(_this,key,value);
}

void ios_hash_put_float(IosHash *_this, const char *key, float f) {
    HashValue value;
    value.f = f;
    ios_hash_put(_this,key,value);
}

void ios_hash_put_ptr(IosHash *_this, const char *key, void *ptr) {
    HashValue value;
    value.ptr = ptr;
    ios_hash_put(_this,key,value);
}

void ios_hash_foreach(IosHash *_this, HashMapAction *action)
{
    gg_str_hashmap::iterator it = _this->root.begin();
    while (it != _this->root.end()) {
        action->action(&((*it).second));
        ++ it;
    }
}

void HashMap::foreach(HashMapAction *action)
{
  ios_hash_foreach(hash, action);
}

}

#if 0
#include <cstring>
#include <cstdlib>
#include <cstdio>
using namespace std;
namespace ios_fc {

#include "bf_hashmap.h"

struct IOS_HASH {
    Hashmap root;
};

/**
 * Copyright (c) 2004 JC Hoelt.
 */

IosHash *ios_hash_new(unsigned int initial_size) {
	IosHash *_this = (IosHash*)malloc(sizeof(IosHash));
	hashmap_open(&_this->root, initial_size);
	return _this;
}

void ios_hash_free(IosHash *_this) {
    hashmap_close(&_this->root);
	free(_this);
}

IosHash *ios_hash_inew(unsigned int initial_size) {
	IosHash *_this = (IosHash*)malloc(sizeof(IosHash));
	hashmap_open(&_this->root, initial_size);
	return _this;
}

void ios_hash_ifree(IosHash *_this) {
    hashmap_close(&_this->root);
	free(_this);
}

void ios_hash_put(IosHash *_this, const char *key, HashValue value) {
    if (_this == NULL) return;
    hashmap_put(&_this->root, (void*)key, NULL, &value, (&value) + 1);
}

HashValue *ios_hash_get(IosHash *_this, const char *key) {
    if (_this == NULL) return NULL;
    return (HashValue*)hashmap_get(&_this->root, (void*)key, NULL);
}

void ios_hash_remove (IosHash *_this, const char *key) {
    printf("hashmap_remove(&_this->root, (void*)key, NULL)\n");
}

void ios_hash_put_int(IosHash *_this, const char *key, int i) {
    HashValue value;
    value.i = i;
    ios_hash_put(_this,key,value);
}

void ios_hash_put_float(IosHash *_this, const char *key, float f) {
    HashValue value;
    value.f = f;
    ios_hash_put(_this,key,value);
}

void ios_hash_put_ptr(IosHash *_this, const char *key, void *ptr) {
    HashValue value;
    value.ptr = ptr;
    ios_hash_put(_this,key,value);
}


void ios_hash_iput(IosHash *_this, int key, HashValue value) {
    hashmap_put(&_this->root, (void*)&key, (void*)((&key) + 1), &value, (&value) + 1);
}

HashValue *ios_hash_iget(IosHash *_this, int key) {
    if (_this == NULL) return NULL;
    return (HashValue*)hashmap_get(&_this->root, (void*)&key, (void*)((&key) + 1));
}

void ios_hash_iremove (IosHash *_this, int key) {
    printf("hashmap_remove(&_this->root, (void*)&key, (void*)((&key) + 1))\n");
}

void ios_hash_iput_int(IosHash *_this, int key, int i) {
    HashValue value;
    value.i = i;
    ios_hash_iput(_this, key, value);
}

void ios_hash_iput_float(IosHash *_this, int key, float f) {
    HashValue value;
    value.f = f;
    ios_hash_iput(_this,key,value);
}

void ios_hash_iput_ptr(IosHash *_this, int key, void *ptr) {
    HashValue value;
    value.ptr = ptr;
    ios_hash_iput(_this,key,value);
}

void ios_hash_foreach(IosHash *_this, HashMapAction *action)
{
    iter i = {0,0};
    HashValue *value;
    while (hashmap_iterate(&_this->root, &i, (void**)&value)) {
        action->action(value);
    }
}

void HashMap::foreach(HashMapAction *action)
{
  ios_hash_foreach(hash, action);
}

}
#endif

