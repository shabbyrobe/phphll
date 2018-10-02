--TEST--
hll_info()
--FILE--
<?php
{ // func
    $hll = hll_create();
    var_dump(hll_info($hll));
}

{ // OO truthy
    $hll = new HyperLogLog();
    var_dump($hll->info());
}

--EXPECT--
array(1) {
  ["encoding"]=>
  string(5) "dense"
}
array(1) {
  ["encoding"]=>
  string(5) "dense"
}
