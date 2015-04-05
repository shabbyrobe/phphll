--TEST--
hll_add() - stringable objects
--FILE--
<?php
require 'helpers.inc';

class StringableObject
{
    public $word;

    function __construct($word)
    {
        $this->word = $word;
    }

    function __toString()
    {
        return $this->word;
    }
}

{ // func
    $hll = hll_create();
    words_each('hll_add', array($hll));
    $count = hll_count($hll);
    var_dump(within_tolerance(WORDS_UNIQUE, $count));
}

{ // OO
    $hll = new HyperLogLog();
    words_each('hll_add_obj', array($hll));
    $count = hll_count($hll);
    var_dump(within_tolerance(WORDS_UNIQUE, $count));
}

--EXPECT--
bool(true)
bool(true)
