#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static size_t growrate = 8192;

static int readln(FILE *stream, char delim, char **buffer, size_t *bufsize)
{
    int c;

    size_t pos = 0;
    do {
        c = getc(stream);
        if (c != EOF) {
            (*buffer)[pos++] = c;
        }
        if (pos == *bufsize) {
            size_t oldsize = *bufsize;
            *bufsize += growrate;
            *buffer = realloc(*buffer, *bufsize);
            memset(*(buffer) + oldsize, '\0', growrate);
        }
    }
    while (c != delim && !feof(stream));

    (*buffer)[pos] = '\0';

    if (pos > 0) {
        (*buffer)[pos-1] = '\0';
        pos--;
    }
    
    return pos;
}
