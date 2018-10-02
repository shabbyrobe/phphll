--TEST--
HyperLogLog() unserialize()
--FILE--
<?php
require 'helpers.inc';

try {
    unserialize('C:11:"HyperLogLog":11:{alkwefjalwef}');
} catch (Exception $ex) {
    echo $ex->getMessage()."\n";
}

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
