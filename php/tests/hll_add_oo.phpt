--TEST--
HyperLogLog::add() - OO interface
--FILE--
<?php
require 'helpers.inc';

$h = new HyperLogLog();
var_dump($h->add("a") == $h);

$h->add("b")->add("c");
var_dump($h->count());

$h->add("d", $updated1);
var_dump($updated1);

$h->add("d", $updated2);
var_dump($updated2);

--EXPECT--
bool(true)
int(3)
bool(true)
bool(false)
