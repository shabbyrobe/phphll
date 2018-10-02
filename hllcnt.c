#include <inttypes.h>

#include "hyperloglog.h"

#include "utils.c"

int main(void)
{
    size_t bufsize = growrate;
    char *buffer = calloc(bufsize, sizeof(char));
    uint64_t cnt = 0;

    hll hll = hllCreate();
    hllSparseToDense(hll);

    int len = 0;
    while ((len = readln(stdin, '\n', &buffer, &bufsize)) > 0) {
        sds sdsbuf = sdsnewlen(buffer, len);
        // printf("%ld %s\n", sdslen(sdsbuf), sdsbuf);
        cnt++;
        if (pfAdd(hll, sdsbuf) == C_ERR) {
            fprintf(stderr, "NOPE!\n");
            exit(1);
        }
        sdsfree(sdsbuf);
    }

    printf("%"PRIu64" %"PRIu64"\n", cnt, pfCount(hll));
    hllFree(hll);
    free(buffer);
}

