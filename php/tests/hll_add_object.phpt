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

    // Note: prior to 7.4, the hll is modified even when the string conversion
    // fails. 7.4 adds try_convert_string, which solves the problem:
    if (version_compare(PHP_VERSION, '7.4.0') >= 0) {
        var_dump(hll_count($hll));
    } else {
        var_dump(hll_count($hll) - 1);
    }

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

    // See note above about 7.4:
    if (version_compare(PHP_VERSION, '7.4.0') >= 0) {
        var_dump(hll_count($hll));
    } else {
        var_dump(hll_count($hll) - 1);
    }

    $hll->add("Object");
    var_dump($hll->count());
}

--EXPECTF--
func:
int(0)
Caught fatal: Object of class NonStringableObject could not be converted to string
int(0)
int(1)

OO:
int(0)
Object of class NonStringableObject could not be converted to string
int(0)
int(1)
