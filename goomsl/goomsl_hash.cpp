#include "goomsl_hash.h"

#include "google/dense_hash_map" // Time efficient hashmap
using google::dense_hash_map;

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
      || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                               +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

#define EMPTY_KEY (NULL)

// by Paul Hsieh
// http://www.azillionmonkeys.com/qed/hash.html
struct SuperFastHashString {
    size_t operator()(const char *data) const {
        if (data == EMPTY_KEY) return 0;
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
                (s2 != EMPTY_KEY) && strcmp(s1, s2) == 0);
    }
};
typedef dense_hash_map<const char*, GHashValue, SuperFastHashString, EqualString> gg_str_hashmap;

struct GOOM_HASH {
    gg_str_hashmap root;
};

/**
 * Copyright (c) 2004 JC Hoelt.
 */

GoomHash *goom_hash_new() {
	GoomHash *_this = new GOOM_HASH;
    _this->root.set_empty_key(EMPTY_KEY);
	return _this;
}

void goom_hash_free(GoomHash *_this) {
    gg_str_hashmap::iterator it = _this->root.begin();
    while (it != _this->root.end()) {
        free((void*)(*it).first);
        ++ it;
    }
	delete _this;
}

void goom_hash_clear(GoomHash *_this) {
    _this->root.clear();
}

void goom_hash_put(GoomHash *_this, const char *key, GHashValue value) {
    if (_this == NULL) return;
    gg_str_hashmap::iterator it = _this->root.find(key);
    if (it == _this->root.end()) {
        int len    = strlen(key);
        char *key2 = (char*)malloc(len+1);
        memcpy(key2,key,len+1);
        key2[len] = 0;
        _this->root[key2] = value;
    }
    else {
        _this->root[key] = value;
    }
}

GHashValue *goom_hash_get(GoomHash *_this, const char *key) {
    if (_this == NULL) return NULL;
    gg_str_hashmap::iterator it = _this->root.find(key);
    if (it == _this->root.end()) {
        return NULL;
    }
    else if ((*it).first == EMPTY_KEY) {
        return NULL;
    }
    else
        return &((*it).second);
}

void goom_hash_put_int(GoomHash *_this, const char *key, int i) {
    GHashValue value;
    value.i = i;
    goom_hash_put(_this,key,value);
}

void goom_hash_put_float(GoomHash *_this, const char *key, float f) {
    GHashValue value;
    value.f = f;
    goom_hash_put(_this,key,value);
}

void goom_hash_put_ptr(GoomHash *_this, const char *key, void *ptr) {
    GHashValue value;
    value.ptr = ptr;
    goom_hash_put(_this,key,value);
}

void goom_hash_for_each(GoomHash *_this, GH_Func func)
{
    gg_str_hashmap::iterator it = _this->root.begin();
    while (it != _this->root.end()) {
        func(_this, (*it).first, &((*it).second));
        ++ it;
    }
}

#if 0
static GoomHashEntry *entry_new(const char *key, GHashValue value) {

  int len = strlen(key);
	GoomHashEntry *entry = (GoomHashEntry*)malloc(sizeof(GoomHashEntry));

	entry->key = (char *)malloc(len+1);
	memcpy(entry->key,key,len+1);
	entry->value = value;
	entry->lower = NULL;
	entry->upper = NULL;

	return entry;
}

static void entry_free(GoomHashEntry *entry) {
	if (entry!=NULL) {
		entry_free(entry->lower);
		entry_free(entry->upper);
		free(entry->key);
		free(entry);
	}
}

static void entry_put(GoomHashEntry *entry, const char *key, GHashValue value) {
	int cmp = strcmp(key,entry->key);
	if (cmp==0) {
		entry->value = value;
	}
	else if (cmp > 0) {
		if (entry->upper == NULL)
			entry->upper = entry_new(key,value);
		else
			entry_put(entry->upper, key, value);
	}
	else {
		if (entry->lower == NULL)
			entry->lower = entry_new(key,value);
		else
			entry_put(entry->lower, key, value);
	}
}

static GHashValue *entry_get(GoomHashEntry *entry, const char *key) {

	int cmp;
	if (entry==NULL)
		return NULL;
	cmp = strcmp(key,entry->key);
	if (cmp > 0)
		return entry_get(entry->upper, key);
	else if (cmp < 0)
		return entry_get(entry->lower, key);
	else
		return &(entry->value);
}

GoomHash *goom_hash_new() {
	GoomHash *_this = (GoomHash*)malloc(sizeof(GoomHash));
	_this->root = NULL;
  _this->number_of_puts = 0;
	return _this;
}

void goom_hash_free(GoomHash *_this) {
	entry_free(_this->root);
	free(_this);
}

void goom_hash_put(GoomHash *_this, const char *key, GHashValue value) {
  _this->number_of_puts += 1;
	if (_this->root == NULL)
		_this->root = entry_new(key,value);
	else
		entry_put(_this->root,key,value);
}

GHashValue *goom_hash_get(GoomHash *_this, const char *key) {
  if (_this == NULL) return NULL;
	return entry_get(_this->root,key);
}

void goom_hash_put_int(GoomHash *_this, const char *key, int i) {
    GHashValue value;
    value.i = i;
    goom_hash_put(_this,key,value);
}

void goom_hash_put_float(GoomHash *_this, const char *key, float f) {
    GHashValue value;
    value.f = f;
    goom_hash_put(_this,key,value);
}

void goom_hash_put_ptr(GoomHash *_this, const char *key, void *ptr) {
    GHashValue value;
    value.ptr = ptr;
    goom_hash_put(_this,key,value);
}

/* FOR EACH */

static void _goom_hash_for_each(GoomHash *_this, GoomHashEntry *entry, GH_Func func)
{
  if (entry == NULL) return;
  func(_this, entry->key, &(entry->value));
  _goom_hash_for_each(_this, entry->lower, func);
  _goom_hash_for_each(_this, entry->upper, func);
}

void goom_hash_for_each(GoomHash *_this, GH_Func func) {
  _goom_hash_for_each(_this, _this->root, func);
}

int goom_hash_number_of_puts(GoomHash *_this) {
  return _this->number_of_puts;
}
#endif
