--TEST--
hll_merge(), HyperLogLog()->merge() - mixed
--FILE--
<?php
require 'helpers.inc';

$hr1 = hll_create();
hll_add($hr1, ['abc', 'def']);

$ho1 = new HyperLogLog();
$ho1->add(['ghi', 'jkl']);

echo "func, 1 resource, 1 obj, array: ";
var_dump(hll_count(hll_merge([$hr1, $ho1])));

echo "func, 1 resource, 1 obj, args: ";
var_dump(hll_count(hll_merge($hr1, $ho1)));

echo "func, 1 resource, 1 obj, array: ";
var_dump(hll_count(hll_merge([$hr1, $ho1])));

echo "obj, 1 resource, 1 obj, args: ";
var_dump((new HyperLogLog())->merge($hr1, $ho1)->count());

echo "obj, 1 resource, 1 obj, array: ";
var_dump((new HyperLogLog())->merge([$hr1, $ho1])->count());

--EXPECT--
func, 1 resource, 1 obj, array: int(4)
func, 1 resource, 1 obj, args: int(4)
func, 1 resource, 1 obj, array: int(4)
obj, 1 resource, 1 obj, args: int(4)
obj, 1 resource, 1 obj, array: int(4)
