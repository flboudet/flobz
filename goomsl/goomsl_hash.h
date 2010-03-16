#ifndef _GOOMSL_HASH_H
#define _GOOMSL_HASH_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GOOM_HASH GoomHash;

typedef union {
    void *ptr;
    int   i;
    float f;
} GHashValue;

GoomHash *goom_hash_new();
void      goom_hash_free(GoomHash *gh);

void goom_hash_clear(GoomHash *_this);
void goom_hash_put(GoomHash *gh, const char *key, GHashValue value);
GHashValue *goom_hash_get(GoomHash *gh, const char *key);

void goom_hash_put_int  (GoomHash *_this, const char *key, int i);
void goom_hash_put_float(GoomHash *_this, const char *key, float f);
void goom_hash_put_ptr  (GoomHash *_this, const char *key, void *ptr);

typedef void (*GH_Func)(GoomHash *caller, const char *key, GHashValue *value);

void goom_hash_for_each(GoomHash *_this, GH_Func func);
int goom_hash_number_of_puts(GoomHash *_this);

#ifdef __cplusplus
}
#endif

#endif /* _GOOM_HASH_H */
