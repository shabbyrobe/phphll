--TEST--
hll_promote()
--FILE--
<?php
$hll = hll_create(true);
$info = hll_info($hll);
var_dump($info['encoding']);

hll_promote($hll);
$info = hll_info($hll);
var_dump($info['encoding']);

hll_promote($hll);
$info = hll_info($hll);
var_dump($info['encoding']);

--EXPECT--
string(6) "sparse"
string(5) "dense"
string(5) "dense"

