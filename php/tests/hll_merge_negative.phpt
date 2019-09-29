--TEST--
hll_merge(), HyperLogLog()->merge() - ensure merge is non-negative
--FILE--
<?php
require 'helpers.inc';

function hll_rand($n, $range) {
    $hll = hll_create();
    for ($i = 0; $i < $n; $i++) {
        $v = rand(0, $range);
        hll_add($hll, $v);
    }
    return $hll;
}

$h1 = hll_rand(0, 5000);
$h2 = hll_rand(0, 5000);

var_dump(hll_count(hll_merge($h1, $h2)));

--EXPECT--
int(0)
