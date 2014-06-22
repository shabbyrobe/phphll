/*
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __REDIS_H
#define __REDIS_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include "sds.h"

#define REDIS_OK                0
#define REDIS_ERR               -1

#ifdef NDEBUG
#define debug(M, ...)
#else
#include <stdio.h>
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define REDIS_STRING 0

#define redisAssert(_e) ((_e)?(void)0 : (_redisAssert(#_e,__FILE__,__LINE__),_exit(1)))
#define redisPanic(_e) _redisPanic(#_e,__FILE__,__LINE__),_exit(1)

void _redisAssert(char *estr, char *file, int line);
void _redisPanic(char *msg, char *file, int line);

struct redisServer {
    size_t hll_sparse_max_bytes;
};

typedef struct redisObject {
    void *ptr;
} robj;

extern struct redisServer server;

static inline void *createObject(int type, void *ptr) {
    (void) type;
    robj *o = malloc(sizeof(*o));
    o->ptr = ptr;
    return o;
}

#endif
