--TEST--
hll_create() - sparse allowed
--FILE--
<?php
$hll = hll_create(true);
$info = hll_info($hll);
var_dump($info['encoding']);
--EXPECT--
string(6) "sparse"
