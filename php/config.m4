PHP_ARG_ENABLE(hll,
    [Whether to enable the "hll" extension],
    [  enable-hll      Enable "hll" extension support])

if test $PHP_HLL != "no"; then
    PHP_SUBST(HLL_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(hll, ../, HLL_SHARED_LIBADD)

    # define(HLL_CFLAGS, -Wall -Wextra)
    define(HLL_CFLAGS, )
    PHP_NEW_EXTENSION(hll, hll.c, $ext_shared, , HLL_CFLAGS)
fi

