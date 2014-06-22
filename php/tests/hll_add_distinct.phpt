--TEST--
hll_add() - few distinct values
--FILE--
<?php
$hll = hll_create();
hll_add($hll, "abc");
hll_add($hll, "def");
hll_add($hll, "ghi");
hll_add($hll, "jkl");
var_dump(hll_count($hll));
--EXPECT--
int(4)

