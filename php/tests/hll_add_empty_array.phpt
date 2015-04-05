--TEST--
hll_add() - empty array
--FILE--
<?php
{ // func
    $hll = hll_create();
    hll_add($hll, array());
    var_dump(hll_count($hll));
}

{ // OO
    $hll = new HyperLogLog();
    $hll->add(array());
    var_dump($hll->count());
}

--EXPECT--
int(0)
int(0)
