--TEST--
hll_dump() - can load
--FILE--
<?php
require 'helpers.inc';

function check_many_unique($hll)
{
    for ($i=1000000; $i<1010000; $i++) {
        $hll instanceof HyperLogLog ? $hll->add($i) : hll_add($hll, $i);
    }
    return verify($hll, 10000);
}

function check_many_similar($hll)
{
    for ($i=1000000; $i<1001000; $i++) {
        for ($j=0; $j<100; $j++) {
            $hll instanceof HyperLogLog ? $hll->add($j) : hll_add($hll, $j);
        }
    }
    return verify($hll, 100);
}

function check_empty($hll)
{
    return verify($hll, 0);
}

function verify($hll, $approxCount)
{
    $count = hll_count($hll);
    $bin   = $hll instanceof HyperLogLog ? $hll->dump() : hll_dump($hll);
    $check = $hll instanceof HyperLogLog ? new HyperLogLog($bin) : hll_load($bin);
    $result = hll_count($check);

    if ($result != $count) {
        throw new \UnexpectedValueException("Dump/load did not match\n");
    } elseif (!within_tolerance($result, $approxCount)) {
        throw new \UnexpectedValueException("Dump/load was not within tolerance of $approxCount: found $count");
    } else {
        var_dump($result == $count);
    }
}

$checkFuncs = array(
    'check_many_unique',
    'check_many_similar',
    'check_empty',
);

foreach ($checkFuncs as $checkFunc) {
    echo "$checkFunc, func, sparse: ";
    $checkFunc(hll_create(!!'allowSparse'));

    echo "$checkFunc, func, dense: ";
    $checkFunc(hll_create());

    echo "$checkFunc, OO, sparse: ";
    $checkFunc(new HyperLogLog(!!'allowSparse'));

    echo "$checkFunc, OO, dense: ";
    $checkFunc(new HyperLogLog());
}

--EXPECT--
check_many_unique, func, sparse: bool(true)
check_many_unique, func, dense: bool(true)
check_many_unique, OO, sparse: bool(true)
check_many_unique, OO, dense: bool(true)
check_many_similar, func, sparse: bool(true)
check_many_similar, func, dense: bool(true)
check_many_similar, OO, sparse: bool(true)
check_many_similar, OO, dense: bool(true)
check_empty, func, sparse: bool(true)
check_empty, func, dense: bool(true)
check_empty, OO, sparse: bool(true)
check_empty, OO, dense: bool(true)

