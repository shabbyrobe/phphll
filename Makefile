CFLAGS=-Wall -Wextra -g -std=c99 -fPIC -I.
LDLIBS=-lm
OBJECTS=sds.o redis.o hyperloglog.o

.PHONY: php

all: libhll.a libhll.so hllcnt

libhll.a: $(OBJECTS)
	ar rcs $@ $(OBJECTS)
	ranlib $@
	#objcopy --keep-global-symbols=symbols $@

libhll.so: $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)
	#objcopy --keep-global-symbols=symbols $@
	
hllcnt: sds.o libhll.a

junk/test: sds.o libhll.a

junk/hllcnt-hiredis: LDLIBS += -lhiredis
junk/hllcnt-hiredis: redis.o hyperloglog.o

clean:
	rm -f $(OBJECTS) libhll.a libhll.so junk/hllcnt-hiredis junk/test
	cd php && test -f Makefile && make clean || true
	cd php && phpize --clean

php: libhll.a
	cd php && phpize --clean && phpize && ./configure && make clean && make 

php-test: php
	cd php && NO_INTERACTION=1 TEST_PHP_ARGS=--show-diff make test

php-memtest: php
	cat phptests.patch | patch php/run-tests.php
	cd php && echo 's' | TEST_PHP_ARGS=-m make test

