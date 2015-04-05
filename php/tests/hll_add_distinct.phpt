--TEST--
hll_add() - few distinct values
--FILE--
<?php
{ // func
    $hll = hll_create();
    hll_add($hll, "abc");
    hll_add($hll, "def");
    hll_add($hll, "ghi");
    hll_add($hll, "jkl");
    var_dump(hll_count($hll));
}

{ // OO
    $hll = new HyperLogLog();
    $hll->add("abc");
    $hll->add("def");
    $hll->add("ghi");
    $hll->add("jkl");
    var_dump($hll->count());
}

--EXPECT--
int(4)
int(4)

