--TEST--
hll_create() - dense by default
--FILE--
<?php
{ // func
    $hll = hll_create();
    $info = hll_info($hll);
    echo "func implicit: "; var_dump($info['encoding']);

    $hll = hll_create(false);
    $info = hll_info($hll);
    echo "func explicit: "; var_dump($info['encoding']);
}

{ // OO
    $hll = new HyperLogLog();
    $info = $hll->info();
    echo "OO implicit: "; var_dump($info['encoding']);

    $hll = new HyperLogLog(false);
    $info = $hll->info();
    echo "OO explicit: "; var_dump($info['encoding']);
}

--EXPECT--
func implicit: string(5) "dense"
func explicit: string(5) "dense"
OO implicit: string(5) "dense"
OO explicit: string(5) "dense"
