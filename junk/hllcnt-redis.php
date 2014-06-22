<?php
require __DIR__.'/redisent.php';

$redis = new redisent\Redis;
$redis->del('testhll');
if ($redis->pfcount('testhll') != 0)
    throw new \Exception();

$cnt = 0;

$batchSize = isset($argv[1]) ? (int)$argv[1] : 2000;
if ($batchSize <= 0)
    die("Dodgy batch size\n");

if ($batchSize > 1) {
    $t = $redis->pipeline();
    while (!feof(STDIN)) {
        $line = trim(fgets(STDIN));
        if ($line) {
            $cnt++;
            $t->pfadd('testhll', $line);
            if ($cnt > 0 && $cnt % $batchSize == 0) {
                $t->uncork();
                $t = $redis->pipeline();
            }
        }
    }
    $t->uncork();
}
else {
    while (!feof(STDIN)) {
        $cnt++;
        $line = fgets(STDIN);
        $redis->pfadd('testhll', $line);
    }
}

$pfcnt = $redis->pfcount('testhll');
echo "$cnt $pfcnt\n";

