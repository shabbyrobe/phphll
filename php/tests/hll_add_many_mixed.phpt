--TEST--
hll_add() - many different values
--FILE--
<?php
require 'helpers.inc';

$hll = hll_create();

words_each('hll_add', array($hll));

$count = hll_count($hll);

var_dump(hll_within_tolerance(WORDS_UNIQUE, $count));

--EXPECT--
bool(true)
