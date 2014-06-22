--TEST--
hll_add() - memory growth check, string signature
--FILE--
<?php
$hll = hll_create();
$memTolerance = 1000;
$memCheckInterval = 1000;

$memPeak = $memInitial = memory_get_usage();

for ($i = 0; $i < 1000000; $i++) {
    hll_add($hll, "$i$i$i");
    if ($i % $memCheckInterval == 0)
        $memPeak = max($memPeak, memory_get_usage());
}

var_dump($memPeak - $memInitial < $memTolerance);

--EXPECT--
bool(true)
