--TEST--
hll_add() - strings
--FILE--
<?php

echo "Empty string considered identical: ";
$hll = hll_create();
hll_add($hll, "");
hll_add($hll, "");
$cnt = hll_count($hll);
echo "$cnt ".($cnt == 1 ? 'OK' : 'NOPE');

echo "\nEmpty string different to non-empty: ";
$hll = hll_create();
hll_add($hll, "");
hll_add($hll, "a");
$cnt = hll_count($hll);
echo "$cnt ".($cnt == 2 ? 'OK' : 'NOPE');

echo "\nNon-empty strings identical: ";
$hll = hll_create();
hll_add($hll, "a");
hll_add($hll, "a");
$cnt = hll_count($hll);
echo "$cnt ".($cnt == 1 ? 'OK' : 'NOPE');

echo "\nArray of strings: ";
$hll = hll_create();
hll_add($hll, ['a', 'b']);
$cnt = hll_count($hll);
echo "$cnt ".($cnt == 2 ? 'OK' : 'NOPE');

--EXPECT--
Empty string considered identical: 1 OK
Empty string different to non-empty: 2 OK
Non-empty strings identical: 1 OK
Array of strings: 2 OKK
