--TEST--
hll_add() - many distinct values
--FILE--
<?php
require 'helpers.inc';

$hll = hll_create();
$total = 1000000;
for ($i=0; $i<$total; $i++) {
    hll_add($hll, "$i");
}
$count = hll_count($hll);

if (hll_within_tolerance($total, $count)) {
    var_dump(true);
}
else {
    var_dump($total, $count, within_pct($total, $count));
}

--EXPECT--
bool(true)
