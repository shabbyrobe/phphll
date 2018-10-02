--TEST--
HyperLogLog() serialize()
--FILE--
<?php
require 'helpers.inc';

$hll = new HyperLogLog();
$hll->add(array('a', 'b', 'c'));
var_dump($hll->count());

$ser = serialize($hll);
$hll = null;

$hll = unserialize($ser);
var_dump($hll->count());

--EXPECTF--
int(3)
int(3)
