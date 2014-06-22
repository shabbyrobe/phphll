--TEST--
hll_count() - invalid resource
--FILE--
<?php
$hll = false;
hll_count($hll);
--EXPECTF--
Warning: hll_count(): Supplied argument is not a valid HyperLogLog resource in %s
