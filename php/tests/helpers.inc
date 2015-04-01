<?php
function within_pct($a, $b)
{
    $within = ((abs($a - $b) / $b) * 100);
    return $within;
}

function hll_within_tolerance($total, $hllCount)
{
    return within_pct($total, $hllCount) <= 1;
}

function catchable_fatal_handler($errno, $errstr, $errfile, $errline)
{
    if (E_RECOVERABLE_ERROR === $errno) {
        echo "Caught fatal: $errstr";
        return true;
    }
    return false;
}

define("WORDS_UNIQUE", 12238);
define("WORDS_LINES", 72786);

function words_each($callback, $args)
{
    $h = gzopen(__DIR__.'/words.gz', 'r');
    while (!feof($h)) {
        $word = trim(fgets($h));
        $curArgs = $args;
        if ($word) {
            $curArgs[] = $word;
            call_user_func_array($callback, $curArgs);
        }
    }
}
