--TEST--
hll_dump() - can load
--FILE--
<?php
$hll = hll_create(true);
hll_add($hll, "a");
hll_add($hll, "a");
hll_add($hll, "a");
hll_add($hll, "a");

$count = hll_count($hll);

$bin = hll_dump($hll);
$hll2 = hll_load($bin);

$result = hll_count($hll2);

var_dump($result == $count);

--EXPECT--
bool(true)
