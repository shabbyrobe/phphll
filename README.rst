PHP HyperLogLog Extension
=========================

.. image:: https://travis-ci.org/shabbyrobe/phphll.svg

This project lifts the ``hyperloglog.c`` implementation from Redis 2.8.19
(http://redis.io) and wraps it in a PHP7 extension.

It is tested with PHP 7.1 and should work with any version of PHP7. If you need
a version that supports PHP 5, try `this commit
<https://github.com/shabbyrobe/phphll/commit/d3b30b00b5fe30b7da689dea288ec1b144ac5808>`_.

Redis is copyright (c) 2014 Salvatore Sanfilippo. Full license details are found in
``COPYING`` at the root of this project. Further credit is due for the exceptional quality
of both the C code and comments found in the Redis project.

HyperLogLog is neatly described by Salvatore himself in his blog post announcing support
in Redis [1]_:

    HyperLogLog is remarkable as it provides a very good approximation of the cardinality
    of a set even using a very small amount of memory. In the Redis implementation it only
    uses 12kbytes per key to count with a standard error of 0.81%, and there is no limit
    to the number of items you can count, unless you approach 2^64 items (which seems
    quite unlikely).

.. [1] http://antirez.com/news/75

HyperLogLogs are magical! See for yourself... the memory usage in the following example
stays stubbornly and hilariously low:

.. code-block:: php
    
    <?php
    $h = new HyperLogLog();
    for ($i=1; $i<100000000; $i++, $h->add($i)) {
        if ($i % 30000 == 0) {
            $c = $h->count();
            printf("\rmem: %d, count: %d, hllcount: %d, diff: %d          ", 
                memory_get_usage(), $i, $c, abs($c - $i));
        }
    }

This extension is intended for situations where you want the flexibility of this data
structure without the overhead of communicating with Redis.


.. contents::
    :depth: 2


Warnings and Disclaimers
------------------------

- No guarantees of thread safety are currently made - Redis itself is designed
  to be run in a single process environment. Assume unsafe until demonstrated otherwise.

- This extension has been tested using PHP 7.1 on a 64-bit machine. YMMV - please let me
  know if there are any failing or any missing tests.

- Dumping and loading methods are provided, but if the internal structure of the
  HyperLogLog representation changes in any backwards incompatible way, no guarantees are
  currently made that dumping and loading will work from version to version, and no
  compatibility methods will be provided.

- HyperLogLog objects are not currently cloneable (though this is in the works)

- This no longer works with any PHP 5 version. The last version compatible with PHP 5.4
  and up can be downloaded here: https://github.com/shabbyrobe/phphll/tree/5.4-compatible
  The last version compatible with 5.2 and 5.3 can be downloaded here:
  https://github.com/shabbyrobe/phphll/tree/5.2-compatible


Bogus Benchmarks
----------------

Here are some numbers for the time it takes to use HyperLogLog to calculate the
cardinality using Redis + PHP, Redis + C, and this extension. You can make some pretty
outrageous time savings if you're using HyperLogLog intensively and don't require Redis.

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
    $ cat /tmp/ids | /usr/bin/time -f '%e' php -d 'extension=php/modules/hll.so' junk/hllcnt.php
    67.27

    ## C, direct, one pfAdd() per entry
    $ cat /tmp/ids | /usr/bin/time -f '%e' ./hllcnt 
    5.01


Building / Installing
---------------------

Provided you have all of the relevant PHP development tools available, build using the
following command::

    make php

The resulting library will be in ``php/modules/hll.so``. You can install it like so::

    cd php; sudo make install

Then add ``extension=hll.so`` to your ``php.ini`` file.


Usage
-----

Counting:

.. code-block:: php
    
    <?php
    $hll = new HyperLogLog();
    $hll->add(["abc", "def", "ghi", "abc", "ghi"]);
    var_dump($hll->count());
    // int(3)


Arbitrary size sets, but slightly inaccurate:

.. code-block:: php

    <?php
    $hll = new HyperLogLog();
    for ($i=0; $i<10000000; $i++) {
        $hll->add($i);
    }

    var_dump($hll->count());
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
    $hll = new HyperLogLog($hllDump ?: null);

    $hll->add($item);
    var_dump($hll->count());

    $redis->set('testhll', $hll->dump());


API
---

``HyperLogLog`` class
~~~~~~~~~~~~~~~~~~~~~

The HyperLogLog class has the following features:

- serializable
- fluent

And the following limitations:

- not cloneable (yet)
- not comparable


``HyperLogLog::__construct()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Creates a new HyperLogLog:: 

    HyperLogLog HyperLogLog::__construct ([ bool $canBeSparse = false ])
    HyperLogLog HyperLogLog::__construct ([ string $hllDump ])

``canBeSparse``
    Defaults to ``false``.
    
    If ``true``, the underlying HyperLogLog will be allowed to encode as a sparse
    representation. This takes less memory, but is considerably slower to work with. Once
    the structure exceeds a certain internally defined size, it is promoted automatically.

    See https://github.com/antirez/redis/blob/2.8.11/src/hyperloglog.c#L56 for some
    excellent documentation on the sparse and dense encodings used by Redis.

``hllDump``
    A string created by ``HyperLogLog::dump()``. Used to reconstruct a HyperLogLog from a
    dumped representation.


``HyperLogLog::add()``
^^^^^^^^^^^^^^^^^^^^^^

Adds one or more scalar elements to a HyperLogLog object, returning the HyperLogLog::

    HyperLogLog HyperLogLog::add( scalar $value , [ bool &$updated ])
    HyperLogLog HyperLogLog::add( array $values , [ bool &$updated ])

``$updated`` will be set to ``true`` if the underlying data structure was updated,
``false`` otherwise. This does not indicate whether the count was updated, only that the
internal data structure has changed. You do not need to pass this.

.. warning: this API is *unstable*. It may end up returning $updated instead of taking it
   by reference. It may also allow a variable argument variant.

.. code-block:: php

    <?php
    $hll = new HyperLogLog();
    $hll->add('a')->add(['b', 'c', 'd'])->add('d', $updated);
    var_dump($updated);
    // bool(false)


``HyperLogLog::count()``
^^^^^^^^^^^^^^^^^^^^^^^^

Returns the cardinality of the HyperLogLog::

    int HyperLogLog::count()

You can use ``hll_count($hll1, $hll2)`` to perform a merged count:

.. code-block:: php

    <?php
    $hll1 = (new HyperLogLog())->add(['a', 'b']);
    $hll2 = (new HyperLogLog())->add(['a', 'c']);
    var_dump(hll_count($hll1, $hll2));
    // int(3)


``HyperLogLog::merge()``
^^^^^^^^^^^^^^^^^^^^^^^^

Merges the HyperLogLog with one or more existing HyperLogLogs, returning the called
HyperLogLog::

    HyperLogLog HyperLogLog::merge ( mixed $hyperLogLog [ , mixed $... ])
    HyperLogLog HyperLogLog::merge ( array $hyperLogLogs )

.. code-block:: php

    <?php
    $hll1 = (new HyperLogLog())->add(['foo', 'bar']);
    $hll2 = (new HyperLogLog())->add(['bar', 'baz']);

    assert($hll1->merge($hll2)->count() == 3);


The passed HyperLogLogs can be in either Object or resource form:

.. code-block:: php

    <?php    
    $hll1 = new HyperLogLog();
    $hll1->merge([hll_create(), new HyperLogLog()]);


``HyperLogLog::promote()``
^^^^^^^^^^^^^^^^^^^^^^^^^^

Ensures a HyperLogLog has a dense representation::

    HyperLogLog HyperLogLog::promote( void )

If the HyperLogLog is already dense, this function does nothing.

See https://github.com/antirez/redis/blob/2.8.11/src/hyperloglog.c#L56 for some excellent
documentation on the sparse and dense encodings used by Redis.


``HyperLogLog::dump()``
^^^^^^^^^^^^^^^^^^^^^^^

Dumps a binary representation of the underlying HyperLogLog::

    string HyperLogLog::dump( void );

 The return type will be a string, but the string will contain binary data and contains
 ``\0`` characters that should not be ignored.

.. warning:: This is a direct dump of Redis' internal representation of the HyperLogLog.
    The dump can only be guaranteed to work with the version of Redis from which the
    ``hyperloglog.c`` file was taken. It should not be used for anything permanent.

You can pass the resulting dump back into the constructor:

.. code-block:: php
    
    <?php
    $h1 = (new HyperLogLog())->add(['a', 'b', 'c']);
    assert($h1->count() == 3);

    $h2 = new HyperLogLog($h->dump());
    assert($h2->count() == 3);


``HyperLogLog::info()``
^^^^^^^^^^^^^^^^^^^^^^^

Returns an array of information about a HyperLogLog::

    array HyperLogLog::info ( void )

.. code-block:: php

    <?php
    $h = hll_create();
    var_dump(hll_info($h));
    // array(1) {
    //    ["encoding"]=>string(5) "dense"
    // }


``HyperLogLog->hll``
^^^^^^^^^^^^^^^^^^^^

The HyperLogLog resource used by the class. This can be manipulated using the procedural
functions documented below.


Procedural Interface
~~~~~~~~~~~~~~~~~~~~

Each method on HyperLogLog has a procedural analog that operates on a HyperLogLog resource
rather than an instance of the HyperLogLog class::

    resource hll_create ([ bool $allowSparse = false ])

    bool hll_add ( resource $hll , scalar $value )
    bool hll_add ( resource $hll , array $values )

    int hll_count ( mixed $hll [ , mixed $... ])

    resource hll_merge( mixed $hyperLogLog1 , mixed $hyperLogLog2 [ , mixed $... ])
    resource hll_merge( array $hyperLogLogs )

    void hll_promote ( resource $hll )

    string hll_dump ( resource $hll )

    resource hll_load ( string $hllDump )

    array hll_info ( resource $hll )


All signatures in the above API that accept a ``mixed`` hyperloglog parameter rather than
a ``resource`` parameter will accept either the resource or the object version, or a
mixture thereof:

.. code-block:: php

    <?php
    $h1 = (new HyperLogLog())->add('a');
    $h2 = hll_create();
    hll_add($h2, 'foo');

    assert(hll_count($h1, $h2) == 2);
    

``hll_load``
^^^^^^^^^^^^

Creates a HyperLogLog resource from a string representation created by ``hll_dump`` or
``HyperLogLog::dump()``::

    resource hll_load( string $dump )

Analog of ``new HyperLogLog(hll_dump($hll))``

