--TEST--
hll_add() - empty array
--FILE--
<?php
$hll = hll_create();
hll_add($hll, []);
var_dump(hll_count($hll));
--EXPECT--
int(0)