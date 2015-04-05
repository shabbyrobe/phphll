--TEST--
hll_add() - many distinct values
--FILE--
<?php
require 'helpers.inc';

function check($hll, $addfn)
{
    $total = 100000;
    for ($i=0; $i<$total; $i++) {
        $addfn($hll, "$i");
    }
    $count = hll_count($hll);

    if (within_tolerance($total, $count)) {
        var_dump(true);
    }
    else {
        var_dump($total, $count, within_pct($total, $count));
    }
}

check(hll_create(), 'hll_add');
check(new HyperLogLog, 'hll_add_obj');

--EXPECT--
bool(true)
bool(true)
