<?php
require __DIR__.'/redisent.php';

$redis = new redisent\Redis;

$hllDump = $redis->get('testhll');

if (!$hllDump)
    $hll = hll_create();
else
    $hll = hll_load($hllDump);

$cnt = 0;
while (!feof(STDIN)) {
    $line = trim(fgets(STDIN));
    if ($line) {
        hll_add($hll, $line);
        $cnt++;
    }
}

$pfcnt = hll_count($hll);

$redis->set('testhll', hll_dump($hll));

