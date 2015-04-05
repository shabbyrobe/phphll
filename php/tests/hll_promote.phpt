--TEST--
hll_promote()
--FILE--
<?php
function check($hll)
{
    $info = $hll instanceof HyperLogLog ? $hll->info() : hll_info($hll);
    var_dump($info['encoding']);

    $hll instanceof HyperLogLog ? $hll->promote() : hll_promote($hll);
    $info = $hll instanceof HyperLogLog ? $hll->info() : hll_info($hll);
    var_dump($info['encoding']);

    $hll instanceof HyperLogLog ? $hll->promote() : hll_promote($hll);
    $info = $hll instanceof HyperLogLog ? $hll->info() : hll_info($hll);
    var_dump($info['encoding']);
}

echo "func sparse:\n";
check(hll_create(true));

echo "\nfunc dense:\n";
check(hll_create(false));

echo "\nOO sparse:\n";
check(new HyperLogLog(true));

echo "\nOO dense:\n";
check(new HyperLogLog(false));

--EXPECT--
func sparse:
string(6) "sparse"
string(5) "dense"
string(5) "dense"

func dense:
string(5) "dense"
string(5) "dense"
string(5) "dense"

OO sparse:
string(6) "sparse"
string(5) "dense"
string(5) "dense"

OO dense:
string(5) "dense"
string(5) "dense"
string(5) "dense"

