--TEST--
hll_create() - dense by default
--FILE--
<?php
$hll = hll_create();
$info = hll_info($hll);
var_dump($info['encoding']);
--EXPECT--
string(5) "dense"
