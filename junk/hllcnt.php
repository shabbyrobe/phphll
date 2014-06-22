<?php

$hll = hll_create();

$batchSize = isset($argv[1]) ? $argv[1] : 1;

$cnt = 0;

if ($batchSize == 1) {
    while (!feof(STDIN)) {
        $line = trim(fgets(STDIN));
        if ($line) {
            hll_add($hll, $line);
            $cnt++;
        }
    }
}
else {
    $batch = [];
    while (!feof(STDIN)) {
        $line = trim(fgets(STDIN));
        if ($line) {
            $batch[] = $line;
            $cnt++;
        }

        if ($cnt > 0 && $cnt % $batchSize == 0) {
            hll_add($hll, $batch);
            $batch = [];
        }
    }

    if ($batch)
        hll_add($hll, $batch);
}

$pfcnt = hll_count($hll);
echo "$cnt $pfcnt\n";

