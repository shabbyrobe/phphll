--TEST--
hll_add() - resource
--FILE--
<?php
$h = fopen('php://memory', 'r+');

$hll = hll_create();
hll_add($hll, $h);

--EXPECTF--
Warning: hll_add(): Argument could not be converted to string in %s
