--TEST--
hll_add() - resource
--FILE--
<?php
$h = fopen('php://memory', 'r+');

{ // func
    $hll = hll_create();
    hll_add($hll, $h);
}

{ // OO
    $hll = new HyperLogLog();
    try {
        $hll->add($h);
    }
    catch (\HyperLogLogException $hex) {
        echo $hex->getMessage();
    }
}

--EXPECTF--
Warning: hll_add(): Argument could not be converted to string in %s
HyperLogLog::add(): Argument could not be converted to string
