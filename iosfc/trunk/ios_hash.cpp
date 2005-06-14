#include "ios_hash.h"

#include <cstring>
#include <cstdlib>
using namespace std;

namespace ios_fc {

/**
 * Copyright (c) 2004 JC Hoelt.
 */

static IosHashEntry *entry_new(const char *key, HashValue value) {

	IosHashEntry *entry = (IosHashEntry*)malloc(sizeof(IosHashEntry));

	entry->key.skey = (char *)malloc(strlen(key)+1);
	strcpy(entry->key.skey, key);
	entry->value = value;
	entry->lower = NULL;
	entry->upper = NULL;

	return entry;
}

static IosHashEntry *entry_inew(int key, HashValue value) {

	IosHashEntry *entry = (IosHashEntry*)malloc(sizeof(IosHashEntry));

	entry->key.ikey = key;
	entry->value = value;
	entry->lower = NULL;
	entry->upper = NULL;

	return entry;
}

static void entry_free(IosHashEntry *entry) {
	if (entry!=NULL) {
		entry_free(entry->lower);
		entry_free(entry->upper);
		free(entry->key.skey);
		free(entry);
	}
}

static void entry_ifree(IosHashEntry *entry) {
	if (entry!=NULL) {
		entry_ifree(entry->lower);
		entry_ifree(entry->upper);
		free(entry);
	}
}

static void entry_put(IosHashEntry *entry, const char *key, HashValue value) {
	int cmp = strcmp(key,entry->key.skey);
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

static HashValue *entry_get(IosHashEntry *entry, const char *key) {

	int cmp;
	if (entry==NULL)
		return NULL;
	cmp = strcmp(key,entry->key.skey);
	if (cmp > 0)
		return entry_get(entry->upper, key);
	else if (cmp < 0)
		return entry_get(entry->lower, key);
	else
		return &(entry->value);
}

static HashValue *entry_get_like(IosHashEntry *entry, const char *key) {
    int cmp;
    if (entry == NULL)
        return NULL;
    cmp = strncmp(key,entry->key.skey,strlen(key));
    if (cmp > 0)
        return entry_get_like(entry->upper, key);
    else if (cmp < 0)
        return entry_get_like(entry->lower, key);
    else
        return &(entry->value);
}

static void entry_remove(IosHashEntry *entry, IosHashEntry **caller, const char *key) {
    int cmp;
    if (entry == NULL) return;
    cmp = strcmp(key, entry->key.skey);
    if (cmp > 0)
        entry_remove(entry->upper, &(entry->upper), key);
	else if (cmp < 0)
        entry_remove(entry->lower, &entry->lower, key);
	else {
        IosHashEntry *sentry = entry;
        if (entry->lower) {
            *caller = entry->lower;
            if (entry->upper) {
                IosHashEntry *upper = entry->upper;
                entry = entry->lower;
                while(entry->upper) entry = entry->upper;
                entry->upper = upper;
            }
        }
        else
            *caller = entry->upper;
        sentry->lower = sentry->upper = 0;
        entry_free(sentry);
    }
}

static void entry_iremove(IosHashEntry *entry, IosHashEntry **caller, int key) {
    int cmp;
    if (entry == NULL) return;
    cmp = key - entry->key.ikey;
    if (cmp > 0)
        entry_iremove(entry->upper, &(entry->upper), key);
	else if (cmp < 0)
        entry_iremove(entry->lower, &(entry->lower), key);
	else {
        IosHashEntry *sentry = entry;
        if (entry->lower) {
            *caller = entry->lower;
            if (entry->upper) {
                IosHashEntry *upper = entry->upper;
                entry = entry->lower;
                while(entry->upper) entry = entry->upper;
                entry->upper = upper;
            }
        }
        else
            *caller = entry->upper;
        sentry->lower = sentry->upper = 0;
        entry_ifree(sentry);
    }
}

IosHash *ios_hash_new() {
	IosHash *_this = (IosHash*)malloc(sizeof(IosHash));
	_this->root = NULL;
	return _this;
}

void ios_hash_free(IosHash *_this) {
	entry_free(_this->root);
	free(_this);
}

IosHash *ios_hash_inew() {
	IosHash *_this = (IosHash*)malloc(sizeof(IosHash));
	_this->root = NULL;
	return _this;
}

void ios_hash_ifree(IosHash *_this) {
	entry_ifree(_this->root);
	free(_this);
}

void ios_hash_put(IosHash *_this, const char *key, HashValue value) {
	if (_this->root == NULL)
		_this->root = entry_new(key,value);
	else
		entry_put(_this->root,key,value);
}

HashValue *ios_hash_get(IosHash *_this, const char *key) {
	return entry_get(_this->root,key);
}

HashValue *ios_hash_get_like(IosHash *_this, const char *key) {
	return entry_get_like(_this->root,key);
}

void ios_hash_remove (IosHash *_this, const char *key) {
    entry_remove(_this->root, &_this->root, key);
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


static void entry_iput(IosHashEntry *entry, int key, HashValue value) {
	int cmp = key - entry->key.ikey;
	if (cmp==0) {
		entry->value = value;
	}
	else if (cmp > 0) {
		if (entry->upper == NULL)
			entry->upper = entry_inew(key,value);
		else
			entry_iput(entry->upper, key, value);
	}
	else {
		if (entry->lower == NULL)
			entry->lower = entry_inew(key,value);
		else
			entry_iput(entry->lower, key, value);
	}
}

static HashValue *entry_iget(IosHashEntry *entry, int key) {

	int cmp;
	if (entry==NULL)
		return NULL;
	cmp = key - entry->key.ikey;
	if (cmp > 0)
		return entry_iget(entry->upper, key);
	else if (cmp < 0)
		return entry_iget(entry->lower, key);
	else
		return &(entry->value);
}

void ios_hash_iput(IosHash *_this, int key, HashValue value) {
    if (_this->root == NULL)
        _this->root = entry_inew(key,value);
    else
        entry_iput(_this->root,key,value);
}

HashValue *ios_hash_iget(IosHash *_this, int key) {
	return entry_iget(_this->root,key);
}

void ios_hash_iremove (IosHash *_this, int key) {
    entry_iremove(_this->root, &_this->root, key);
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

void ios_hash_foreach(IosHashEntry *entry, HashMapAction *action)
{
  if (entry == NULL) return;
  action->action(&entry->value);
  ios_hash_foreach(entry->lower, action);
  ios_hash_foreach(entry->upper, action);
}

void HashMap::foreach(HashMapAction *action)
{
  ios_hash_foreach(hash->root, action);
}

}

