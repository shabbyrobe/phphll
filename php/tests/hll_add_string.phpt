--TEST--
hll_add() - strings
--FILE--
<?php
require 'helpers.inc';

function check($newfn, $addfn)
{
    echo "Empty string considered identical: ";
    $hll = $newfn();
    $addfn($hll, "");
    $addfn($hll, "");
    $cnt = hll_count($hll);
    echo "$cnt ".($cnt == 1 ? 'OK' : 'NOPE');

    echo "\nEmpty string different to non-empty: ";
    $hll = $newfn();
    $addfn($hll, "");
    $addfn($hll, "a");
    $cnt = hll_count($hll);
    echo "$cnt ".($cnt == 2 ? 'OK' : 'NOPE');

    echo "\nNon-empty strings identical: ";
    $hll = $newfn();
    $addfn($hll, "a");
    $addfn($hll, "a");
    $cnt = hll_count($hll);
    echo "$cnt ".($cnt == 1 ? 'OK' : 'NOPE');

    echo "\nArray of strings: ";
    $hll = $newfn();
    $addfn($hll, array('a', 'b'));
    $cnt = hll_count($hll);
    echo "$cnt ".($cnt == 2 ? 'OK' : 'NOPE');
}

echo "func:\n";
check('hll_create', 'hll_add');

echo "\n\nOO:\n";
check('hll_create_obj', 'hll_add_obj');


--EXPECT--
func:
Empty string considered identical: 1 OK
Empty string different to non-empty: 2 OK
Non-empty strings identical: 1 OK
Array of strings: 2 OK

OO:
Empty string considered identical: 1 OK
Empty string different to non-empty: 2 OK
Non-empty strings identical: 1 OK
Array of strings: 2 OK

