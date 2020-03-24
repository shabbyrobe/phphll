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
    var_dump(hll_count($hll));
    try {
        hll_add($hll, new NonStringableObject());
    }
    catch (Throwable $e) { // 7.4
        echo "Caught fatal: {$e->getMessage()}";
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
    var_dump(hll_count($hll));

    try {
        $hll->add(new NonStringableObject());
    }
    catch (\HyperLogLogException $hex) {
        echo $hex->getMessage()."\n";
    }
    catch (\Throwable $ex) { // 7.4
        echo $ex->getMessage()."\n";
    }

    $hll->add("Object");
    var_dump($hll->count());
}

--EXPECTF--
func:
int(0)
Caught fatal: Object of class NonStringableObject could not be converted to string
int(1)
int(1)

OO:
int(0)
Object of class NonStringableObject could not be converted to string
int(1)
