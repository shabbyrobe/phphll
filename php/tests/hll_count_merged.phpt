--TEST--
hll_count() - count merged
--FILE--
<?php
{ // func
    $h1 = hll_create();
    hll_add($h1, 'abc');
    hll_add($h1, 'def');

    $h2 = hll_create();
    hll_add($h2, 'def');
    hll_add($h2, 'ghi');

    $h3 = hll_create();
    hll_add($h3, 'ghi');
    hll_add($h3, 'jkl');

    echo 'func, $h1, $h2: '; var_dump(hll_count($h1, $h2));
    echo 'func, $h1, $h2, $h3: '; var_dump(hll_count($h1, $h2, $h3));
}

{ // OO
    $h1 = new HyperLogLog();
    $h1->add('abc');
    $h1->add('def');

    $h2 = new HyperLogLog();
    $h2->add('def');
    $h2->add('ghi');

    $h3 = new HyperLogLog();
    $h3->add('ghi');
    $h3->add('jkl');

    echo 'OO, $h1, $h2: '; var_dump(hll_count($h1, $h2));
    echo 'OO, $h1, $h2, $h3: '; var_dump(hll_count($h1, $h2, $h3));
}

--EXPECT--
func, $h1, $h2: int(3)
func, $h1, $h2, $h3: int(4)
OO, $h1, $h2: int(3)
OO, $h1, $h2, $h3: int(4)
