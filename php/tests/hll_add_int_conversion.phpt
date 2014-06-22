--TEST--
hll_add() - integer and string considered identical
--FILE--
<?php
$hll = hll_create();

hll_add($hll, "100");
hll_add($hll, 100);
var_dump(hll_count($hll));

--EXPECT--
int(1)

