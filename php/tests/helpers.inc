<?php
function within_pct($a, $b)
{
    if ($a == 0 && $b == 0) {
        return 0;
    }
    list ($a, $b) = array(min($a, $b), max($a, $b));
    $within = ((abs($a - $b) / $b) * 100);
    return $within;
}

function within_tolerance($total, $hllCount)
{
    return within_pct($total, $hllCount) <= 1;
}

function error_handler($errno, $errstr, $errfile, $errline)
{
    echo "$errno: $errstr @ $errfile:$errline\n";
    return true;
}

function eh_catch_recoverable($errno, $errstr, $errfile, $errline)
{
    if (E_RECOVERABLE_ERROR === $errno) {
        echo "Caught fatal: $errstr";
        return true;
    }
    return false;
}

function hll_add_obj($h, $i)
{
    return $h->add($i);
}

function hll_create_obj()
{
    return new HyperLogLog();
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

