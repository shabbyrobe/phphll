--TEST--
hll_add() - non-stringable objects should fail
--FILE--
<?php
require 'helpers.inc';

class NonStringableObject{}

echo "func:\n"; {
    // Outputs the error, allows the script to continue
    set_error_handler('eh_catch_recoverable');

    $hll = hll_create();
    hll_add($hll, new NonStringableObject());
    var_dump(hll_count($hll));

    // Should be equivalent to string 'Object'. Ideally you would let your script
    // die before this anyway.
    hll_add($hll, "Object");
    var_dump(hll_count($hll));

    restore_error_handler();
}

echo "\nOO:\n"; {
    $hll = new HyperLogLog();

    try {
        $hll->add(new NonStringableObject());
    }
    catch (\HyperLogLogException $hex) {
        echo $hex->getMessage()."\n";
    }

    $hll->add("Object");
    var_dump($hll->count());
}

--EXPECTF--
func:
Caught fatal: Object of class NonStringableObject could not be converted to string
Notice: Object of class NonStringableObject to string conversion in %s
int(1)
int(1)

OO:

Notice: Object of class NonStringableObject to string conversion in %s
Object of class NonStringableObject could not be converted to string
int(1)
