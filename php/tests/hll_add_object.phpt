--TEST--
hll_add() - non-stringable objects should fail
--FILE--
<?php
require 'helpers.inc';
set_error_handler('catchable_fatal_handler');

class NonStringableObject{}

$hll = hll_create();
hll_add($hll, new NonStringableObject());

var_dump(hll_count($hll));

// Should be equivalent to string 'Object'. Ideally you would let your script
// die before this anyway.
hll_add($hll, "Object");
var_dump(hll_count($hll));

--EXPECTF--
Caught fatal: Object of class NonStringableObject could not be converted to string
Notice: Object of class NonStringableObject to string conversion in %s
int(1)
int(1)
