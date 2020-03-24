--TEST--
hll_add() - non-stringable objects should fail
--FILE--
<?php
require 'helpers.inc';

class NonStringableObject{}

if (!interface_exists('Throwable')) {
    class Throwable extends Exception {}
}

echo "func:\n"; {
    // Outputs the error, allows the script to continue
    set_error_handler('eh_catch_recoverable');

    $hll = hll_create();
    try {
        hll_add($hll, new NonStringableObject());
    } catch (Throwable $e) {
        echo "Caught fatal: {$e->getMessage()}";
        return true;
    }
    echo "\n";
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
int(1)
int(1)

OO:
Object of class NonStringableObject could not be converted to string
int(1)
