#ifndef __HYPERLOGLOG_H
#define __HYPERLOGLOG_H
#include "redis.h"

#define HLL_OK    0
#define HLL_ERR  -1

#define HLL_DENSE 0 /* Dense encoding. */
#define HLL_SPARSE 1 /* Sparse encoding. */

struct hllhdr {
    char magic[4];      /* "HYLL" */
    uint8_t encoding;   /* HLL_DENSE or HLL_SPARSE. */
    uint8_t notused[3]; /* Reserved for future use, must be zero. */
    uint8_t card[8];    /* Cached cardinality, little endian. */
    uint8_t registers[]; /* Data bytes. */
};

typedef robj * hll;

hll hllCreate(void);
int hllLoad(hll *h, sds raw);
int hllSparseToDense(hll h);
void hllFree(hll h);
sds hllRaw(hll h);

int pfAdd(hll h, sds ele);
int pfAddMany(hll h, sds ele[], size_t ele_len);
uint64_t pfCount(hll h);
uint64_t pfCountMerged(hll hlls[], size_t nmemb);
int pfMerge(hll target, hll sources[], size_t nmemb);

#endif

