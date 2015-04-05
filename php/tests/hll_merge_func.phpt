--TEST--
hll_merge()
--FILE--
<?php
require 'helpers.inc';

$h1 = hll_create();
hll_add($h1, 'abc');
hll_add($h1, 'def');

$h2 = hll_create();
hll_add($h2, 'def');
hll_add($h2, 'ghi');

$h3 = hll_create();
hll_add($h3, 'ghi');
hll_add($h3, 'jkl');

echo "func, 2-way, args:  ";
var_dump(hll_count(hll_merge($h1, $h2)));

echo "func, 2-way, array: ";
var_dump(hll_count(hll_merge(array($h1, $h2))));

echo "func, 3-way, args:  ";
var_dump(hll_count(hll_merge($h1, $h2, $h3)));

echo "func, 3-way, array: ";
var_dump(hll_count(hll_merge(array($h1, $h2, $h3))));


--EXPECT--
func, 2-way, args:  int(3)
func, 2-way, array: int(3)
func, 3-way, args:  int(4)
func, 3-way, array: int(4)
