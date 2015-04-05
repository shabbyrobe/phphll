--TEST--
hll_create() - sparse allowed
--FILE--
<?php
{ // func
    $hll = hll_create(true);
    $info = hll_info($hll);
    var_dump($info['encoding']);
}

{ // OO
    $hll = new HyperLogLog(true);
    $info = $hll->info();
    var_dump($info['encoding']);
}

--EXPECT--
string(6) "sparse"
string(6) "sparse"
