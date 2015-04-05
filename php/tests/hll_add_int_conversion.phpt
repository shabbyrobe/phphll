--TEST--
hll_add() - integer and string considered identical
--FILE--
<?php
{ // func
    $hll = hll_create();
    hll_add($hll, "100");
    hll_add($hll, 100);
    var_dump(hll_count($hll));
}

{ // OO
    $hll = new HyperLogLog();
    $hll->add("100");
    $hll->add(100);
    var_dump($hll->count());
}

--EXPECT--
int(1)
int(1)
