--TEST--
hll_add() - memory growth check, array signature
--FILE--
<?php
$hll = hll_create();
$memTolerance = 1000;
$memCheckInterval = 1000;

$memPeak = $memInitial = memory_get_usage();

for ($i = 0; $i < 200000; $i++) {
    hll_add($hll, array("1-$i$i$i", "2-$i$i$i", "3-$i$i$i"));
    if ($i % $memCheckInterval == 0)
        $memPeak = max($memPeak, memory_get_usage());
}

$diff = $memPeak - $memInitial;
if ($diff < $memTolerance) {
    var_dump(true);
}
else {
    echo "Memory failed: expected $memTolerance, found $diff";
}

--EXPECT--
bool(true)
