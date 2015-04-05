--TEST--
hll_add() - float and string considered identical
--FILE--
<?php
{ // func
    $hll = hll_create();
    hll_add($hll, "100.1");
    hll_add($hll, 100.1);
    var_dump(hll_count($hll));
}

{ // OO
    $hll = new HyperLogLog;
    $hll->add("100.1");
    $hll->add(100.1);
    var_dump($hll->count());
}

--EXPECT--
int(1)
int(1)
