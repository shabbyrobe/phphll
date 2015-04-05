--TEST--
hll_add() - memory growth check
--FILE--
<?php
require 'helpers.inc';

function check($name, $hll, $addfn)
{
    $memTolerance = 1000;
    $memCheckInterval = 1000;
    $additions = 50000;

    $memPeak = $memInitial = memory_get_usage();

    for ($i = 0; $i < $additions; $i++) {
        $addfn($hll, $i);
        if ($i % $memCheckInterval == 0) {
            $memPeak = max($memPeak, memory_get_usage());
        }
    }

    $diff = $memPeak - $memInitial;
    if ($diff < $memTolerance) {
        echo "$name: "; var_dump(true);
    } else {
        echo "Memory failed: expected $memTolerance, found $diff";
    }
};

function hll_add_array     ($h, $i) { hll_add($h, array("1-$i", "2-$i", "3-$i")); }
function hll_add_obj_array ($h, $i) { $h->add(    array("1-$i", "2-$i", "3-$i")); }

function hll_add_string($h, $i)     { hll_add($h, $i); }
function hll_add_obj_string($h, $i) { $h->add($i); }

check("Procedural, array", hll_create(), 'hll_add_array');
check("OO, array", new HyperLogLog, 'hll_add_obj_array');
check("Procedural, string", hll_create(), 'hll_add_string');
check("OO, string", new HyperLogLog, 'hll_add_obj_string');

--EXPECT--
Procedural, array: bool(true)
OO, array: bool(true)
Procedural, string: bool(true)
OO, string: bool(true)
