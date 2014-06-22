#include <stdlib.h>
#include <stdint.h>
#include <hiredis/hiredis.h>
#include "../utils.c"

int main(int argc, char *argv[])
{
    int ret = 1;

    size_t bufSize = 8192;
    int batchSize = 2000;

    if (argc > 1) {
        batchSize = atoi(argv[1]);
        if (batchSize <= 0) {
            fprintf(stderr, "Dodgy batch size\n");
            exit(1);
        }
    }

    char *buffer = calloc(bufSize, sizeof(char));
    uint64_t cnt = 0;
    redisReply *reply = NULL;

    redisContext *c = redisConnectUnix("/var/lib/redis/redis.sock");
    if (c != NULL && c->err) {
        printf("Error: %s\n", c->errstr);
        goto cleanup;
    }

    reply = redisCommand(c, "DEL testhll");
    if (reply == NULL) {
        printf("Failed: %s\n", c->errstr);
        goto cleanup;
    }
    freeReplyObject(reply);

    int batchPos = 0;
    int len = 0;
    while ((len = readln(stdin, '\n', &buffer, &bufSize)) > 0) {
        cnt++;
        redisAppendCommand(c, "PFADD testhll %s", buffer);

        batchPos++;
        if (batchSize == batchPos) {
            batchPos = 0;
            for (int j = 0; j < batchSize; j++) {
                redisGetReply(c, (void **)&reply);

                if (reply == NULL) {
                    printf("Failed: %s\n", c->errstr);
                    goto cleanup;
                }
                else if (reply->type == REDIS_REPLY_ERROR) {
                    printf("Failed: %s\n", reply->str);
                    goto cleanup;
                }
                freeReplyObject(reply);
            }
        }
    }

    if (batchPos > 0) {
        for (int j = 0; j < batchPos; j++) {
            redisGetReply(c, (void **)&reply);

            if (reply == NULL) {
                printf("Failed: %s\n", c->errstr);
                goto cleanup;
            }
            else if (reply->type == REDIS_REPLY_ERROR) {
                printf("Failed: %s\n", reply->str);
                goto cleanup;
            }
            freeReplyObject(reply);
        }
    }
    

    reply = redisCommand(c, "PFCOUNT testhll");
    if (reply == NULL) {
        printf("Failed: %s\n", c->errstr);
        goto cleanup;
    }

    printf("%ld %lld\n", cnt, reply->integer);

    ret = 0;

cleanup:
    if (reply != NULL) freeReplyObject(reply);
    free(buffer);
    redisFree(c);

    return ret;
}
