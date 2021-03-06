#ifndef SPLAYTREE_H
#define SPLAYTREE_H

#ifndef WITH_SPLAYTREE
/*#define WITH_SPLAYTREE*/
#endif

#include "dmp.h"
#include "env.h"

typedef int (*SplayTree_CmpFunc)(
			    void *info,
			    const void *,
			    const void *);

typedef struct SplayTree_t SplayTree_t;

SplayTree_t *SplayTree_New(SplayTree_CmpFunc cmp, void *info);
void SplayTree_Free(SplayTree_t *t);
int  SplayTree_isEmpty(SplayTree_t *t);
int  SplayTree_count(SplayTree_t *t);
int  SplayTree_insert(SplayTree_t *t, const void *key, const void *value);
const void *SplayTree_remove(SplayTree_t *t, const void *key);
const void *SplayTree_find(SplayTree_t *t, const void *key);
#define SplayTree_find_prev SplayTree_findGreatestLessThan
#define SplayTree_find_next SplayTree_findSmallestGreaterThan
const void *SplayTree_findGreatestLessThan(SplayTree_t *t, const void *key);
const void *SplayTree_findSmallestGreaterThan(SplayTree_t *t, const void *key);
const void *SplayTree_first(SplayTree_t *t);
const void *SplayTree_peek(SplayTree_t *t);
int SplayTree_strcmp(void *info, const void *key1, const void *key2);

#endif //SPLAYTREE_H
