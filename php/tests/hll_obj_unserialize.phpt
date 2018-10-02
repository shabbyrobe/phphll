--TEST--
HyperLogLog() unserialize()
--FILE--
<?php
require 'helpers.inc';

/*
// Disabled thanks to PHP 7.2 changing the way this is handled.
// < 7.2: "Could not unserialize HLL"
// >=7.2: "Notice: unserialize(): Error at offset 23 of 36 bytes in /home/travis/build/shabbyrobe/phphll"
//
try {
    unserialize('C:11:"HyperLogLog":11:{alkwefjalwef}');
} catch (Exception $ex) {
    echo $ex->getMessage()."\n";
}
*/

try {
    unserialize('C:11:"HyperLogLog":11:{s:4:"HYLL";}');
} catch (Exception $ex) {
    echo $ex->getMessage()."\n";
}

try {
    $hll = new HyperLogLog("blah blah junk");
} catch (HyperLogLogException $ex) {
    echo $ex->getMessage()."\n";
}

--EXPECTF--
Could not unserialize HLL
Could not create HLL from unserialized string
HyperLogLog::__construct(): Supplied HLL dump was invalid
