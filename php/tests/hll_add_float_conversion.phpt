--TEST--
hll_add() - float and string considered identical
--FILE--
<?php
$hll = hll_create();

hll_add($hll, "100.1");
hll_add($hll, 100.1);
var_dump(hll_count($hll));

--EXPECT--
int(1)

