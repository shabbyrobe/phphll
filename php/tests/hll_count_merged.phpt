--TEST--
hll_count() - count merged
--FILE--
<?php
$h1 = hll_create();
hll_add($h1, 'abc');
hll_add($h1, 'def');

$h2 = hll_create();
hll_add($h2, 'def');
hll_add($h2, 'ghi');

$h3 = hll_create();
hll_add($h3, 'ghi');
hll_add($h3, 'jkl');

var_dump(hll_count($h1, $h2));
var_dump(hll_count($h1, $h2, $h3));
var_dump(hll_count(array($h1, $h2, $h3)));

--EXPECT--
int(3)
int(4)
int(4)
