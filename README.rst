Redis HyperLogLog PHP Extension
===============================

This project lifts the ``hyperloglog.c`` implementation from Redis 2.8.11
(http://redis.io) and wraps it in a PHP extension.

Redis is copyright (c) 2014 Salvatore Sanfilippo. Full license details are found in
``COPYING`` at the root of this project. Further credit is due for the exceptional quality
of both the C code and comments found in the Redis project.

HyperLogLog is neatly described by Salvatore himself in his blog post announcing support
in Redis [1]_:

> HyperLogLog is remarkable as it provides a very good approximation of the cardinality of a
> set even using a very small amount of memory. In the Redis implementation it only uses
> 12kbytes per key to count with a standard error of 0.81%, and there is no limit to the
> number of items you can count, unless you approach 2^64 items (which seems quite
> unlikely).

.. [1] http://antirez.com/news/75

This extension is intended for situations where you want the flexibility of this data
structure without the overhead of communicating with Redis.


Warnings and Disclaimers
------------------------

- No guarantees of thread safety are currently made - Redis itself is designed
  to be run in a single process environment and no tests have been run yet against the 
  ZTS build of PHP. Assume unsafe until demonstrated otherwise.

- This extension has only been tested using PHP 5.5 on a 64-bit machine. YMMV - please let
  me know if there are any failing or any missing tests.

- Dumping and loading methods are provided, but if the internal structure of the
  HyperLogLog representation changes in any backwards incompatible way, no guarantees are
  currently made that dumping and loading will work in this scenario.


Bogus Benchmarks
----------------

Here are some numbers for the time it takes to use HyperLogLog to calculate the
cardinality using Redis + PHP, Redis + C, and this extension. Apparently you can make some
pretty outrageous time savings if you're using HyperLogLog intensively and don't require
Redis.

This test was run using a file containing 31793623 random items with a cardinality of 243.
Feel free to point out all the many terrible things I've done wrong here, it's the only
way I'll learn::

    ## PHP, redisent, one PFADD per entry
    $ cat /tmp/ids | /usr/bin/time -f '%e' php junk/hllcnt-redis.php 1
    1504.13

    ## C, hiredis, one PFADD per entry
    $ cat /tmp/ids | /usr/bin/time -f '%e' junk/hllcnt-hiredis 1
    859.44

    ## PHP, redisent, PFADD in batches of 2000
    $ cat /tmp/ids | /usr/bin/time -f '%e' php junk/hllcnt-redis.php 2000
    614.21

    ## C, hiredis, PFADD in batches of 2000
    $ cat /tmp/ids | /usr/bin/time -f '%e' junk/hllcnt-hiredis 2000
    122.82

    ## PHP, hll.so, one pfAdd() per entry
    $ cat /tmp/ids | /usr/bin/time -f '%e' php -d 'extension=php/.libs/hll.so' junk/hllcnt.php
    67.27

    ## C, direct, one pfAdd() per entry
    $ cat /tmp/ids | /usr/bin/time -f '%e' ./hllcnt 
    5.01


Building / Installing
---------------------

Provided you have all of the relevant PHP development tools available, build using the
following command::

    make php

The resulting library will be in ``php/.libs/hll.so``. You can install it like so::

    cd php; sudo make install

Then add ``extension=hll.so`` to your ``php.ini`` file.


Usage
-----

Counting:

.. code-block:: php
    
    <?php
    $hll = hll_create();
    hll_add($hll, ["abc", "def", "ghi", "abc", "ghi"]);
    var_dump(hll_count($hll));
    // int(3)


Arbitrary size sets, but slightly inaccurate:

.. code-block:: php

    <?php
    $hll = hll_create();
    for ($i=0; $i<10000000; $i++)
        hll_add($hll, $i);

    var_dump(hll_count($hll));
    // int(9972088)

    var_dump(memory_get_usage());
    // int(460384)


Interoperating with Redis:

.. warning:: This can only be guaranteed to work if you are using the same version of
   Redis that this was built from.

.. code-block:: php
    
    <?php
    $item = $argv[1];
    $redis = new redisent\Redis;

    // Redis treats HyperLogLogs as simple strings, so we can get and set as we please
    $hllDump = $redis->get('testhll');
    $hll = $hllDump ? hll_load($hllDump) : hll_create();

    hll_add($hll, $item);
    var_dump(hll_count($hll));

    $redis->set('testhll', hll_dump($hll));


API
---

``hll_create``
~~~~~~~~~~~~~~

Creates a HyperLogLog resource::

    resource hll_create([ bool $canBeSparse = false ])

.. warning: this API is *unstable*. It may start creating sparse representations by
   default and it may end up accepting a constant as the first argument.

``canBeSparse``
    Defaults to ``false``.
    
    If ``true``, the underlying HyperLogLog will be allowed to encode as a sparse
    representation. This takes less memory, but is considerably slower to work with. Once
    the structure exceeds a certain internally defined size, it is promoted automatically.

    See https://github.com/antirez/redis/blob/2.8.11/src/hyperloglog.c#L56 for some
    excellent documentation on the sparse and dense encodings used by Redis.


``hll_add``
~~~~~~~~~~~

Adds one or more scalar elements to a HyperLogLog resource::

    bool hll_add( resource $hyperLogLog , scalar $value )
    bool hll_add( resource $hyperLogLog , array $values )

Returns ``true`` if the underlying data structure was updated, ``false`` otherwise. This
does not indicate whether the count was updated. You can safely ignore the return if you
do not wish to be notified of changes.

.. warning: this API is *unstable*. It may end up just returning true on success and false
   on failure. It may also allow a variable argument variant.

.. code-block:: php

   <?php
   $updated = hll_add($hll, "foo");
   $success = hll_add($hll, "foo");
   $success = hll_add($hll, "bar"); // true


``hll_count``
~~~~~~~~~~~~~

Returns the cardinality of one HyperLogLog or many merged HyperLogLogs::

    int hll_count( resource $hyperLogLog [ , resource $... ])
    int hll_count( array $hyperLogLogs )


``hll_merge``
~~~~~~~~~~~~~

Creates a new HyperLogLog by merging together two or more existing HyperLogLogs::

    resource hll_merge( resource $hyperLogLog [ , resource $... ])
    resource hll_merge( array $hyperLogLogs )

.. code-block:: php

    <?php
    $hll1 = hll_create();
    hll_add($hll1, ['foo', 'bar']);

    $hll2 = hll_create();
    hll_add($hll2, ['bar', 'baz']);

    $merged = hll_merge($hll1, $hll2);
    assert(hll_count($merged) == 3);


``hll_promote``
~~~~~~~~~~~~~~~

Ensures a HyperLogLog has a dense representation::

    void hll_promote( resource $hll );

If the HyperLogLog is already dense, this function does nothing.

See https://github.com/antirez/redis/blob/2.8.11/src/hyperloglog.c#L56 for some excellent
documentation on the sparse and dense encodings used by Redis.


``hll_dump``
~~~~~~~~~~~~

Dumps a binary representation of the underlying HyperLogLog::

    string hll_dump( resource $hll );

 The return type will be a string, but the string will contain binary data and contains
 ``\0`` characters that should not be ignored.

.. warning:: This is a direct dump of Redis' internal representation of the HyperLogLog.
    The dump can only be guaranteed to work with the version of Redis from which the
    ``hyperloglog.c`` file was taken. It should not be used for anything permanent.


``hll_load``
~~~~~~~~~~~~

Creates a HyperLogLog from a string representation created by ``hll_dump``::

    resource hll_load( string $dump )

.. warning:: This uses a direct dump of Redis' internal representation of the HyperLogLog.
    The dump can only be guaranteed to work with the version of Redis from which the
    ``hyperloglog.c`` file was taken. It should not be used for anything permanent.


``hll_info``
~~~~~~~~~~~~

Returns an array of information about a HyperLogLog::

    array hll_info( resource $hyperLogLog )

.. code-block:: php

    <?php
    $h = hll_create();
    var_dump(hll_info($h));
    // array(1) {
    //    ["encoding"]=>string(5) "dense"
    // }

