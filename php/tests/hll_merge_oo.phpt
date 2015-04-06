--TEST--
HyperLogLog::merge()
--FILE--
<?php
require 'helpers.inc';

$h1 = new HyperLogLog();
$h1->add('abc');
$h1->add('def');

$h2 = new HyperLogLog();
$h2->add('def');
$h2->add('ghi');

$h3 = new HyperLogLog();
$h3->add('ghi');
$h3->add('jkl');

{ // cumulative
    $hcum = new HyperLogLog();
    foreach (array($h1, $h2, $h3) as $h) {
        $hcum->merge($h);
    }
    printf("OO cumulative: %d\n", $hcum->count());
}

{ // signatures
    $hnew = new HyperLogLog();
    printf(
        "OO, 2-way into blank, args:  %s %s %s %s\n",
        $h1->count(),
        $h2->count(),
        $h3->count(),
        $hnew->merge($h1, $h2)->count()
    );

    printf(
        "OO, 2-way into blank, array: %s %s %s %s\n",
        $h1->count(),
        $h2->count(),
        $h3->count(),
        $hnew->merge(array($h1, $h2))->count()
    );

    printf(
        "OO, 3-way into blank, args:  %s %s %s %s\n",
        $h1->count(),
        $h2->count(),
        $h3->count(),
        $hnew->merge($h1, $h2, $h3)->count()
    );

    printf(
        "OO, 3-way into blank, array: %s %s %s %s\n",
        $h1->count(),
        $h2->count(),
        $h3->count(),
        $hnew->merge(array($h1, $h2, $h3))->count()
    );
}

{ // into itself
    $hself = new HyperLogLog();
    $hself->add('foo');
    $hself->merge($hself);
    echo "Into itself: ".$hself->count()."\n";
}

{ // fluent
    $hflu1 = new HyperLogLog();
    $hflu2 = new HyperLogLog();
    $hflu2->add('a');
    echo "Fluent: "; var_dump($hflu1->merge($hflu2) == $hflu1);
}

--EXPECT--
OO cumulative: 4
OO, 2-way into blank, args:  2 2 2 3
OO, 2-way into blank, array: 2 2 2 3
OO, 3-way into blank, args:  2 2 2 4
OO, 3-way into blank, array: 2 2 2 4
Into itself: 1
Fluent: bool(true)

