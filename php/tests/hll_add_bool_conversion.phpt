--TEST--
hll_add() - bool and string considered identical
--FILE--
<?php
$hll = hll_create();
hll_add($hll, "1");
hll_add($hll, true);
var_dump(hll_count($hll));

$hll = hll_create();
hll_add($hll, "");
hll_add($hll, false);
var_dump(hll_count($hll));

--EXPECT--
int(1)
int(1)
