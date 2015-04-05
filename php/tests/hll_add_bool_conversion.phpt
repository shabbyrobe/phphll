--TEST--
hll_add() - bool and string considered identical
--FILE--
<?php
{ // func truthy
    $hll = hll_create();
    hll_add($hll, "1");
    hll_add($hll, true);
    var_dump(hll_count($hll));
}

{ // OO truthy
    $hll = new HyperLogLog();
    $hll->add("1");
    $hll->add(true);
    var_dump(hll_count($hll));
}

{ // func falsey
    $hll = hll_create();
    hll_add($hll, "");
    hll_add($hll, false);
    var_dump(hll_count($hll));
}

{ // func truthy
    $hll = new HyperLogLog();
    $hll->add("");
    $hll->add(false);
    var_dump($hll->count());
}

--EXPECT--
int(1)
int(1)
int(1)
int(1)
