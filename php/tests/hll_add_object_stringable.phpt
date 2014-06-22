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

$hll = hll_create();

function hll_stringable_add($hll, $word)
{
    $stringable = new StringableObject($word);
    hll_add($hll, $stringable);
}

words_each('hll_add', array($hll));

$count = hll_count($hll);

var_dump(hll_within_tolerance(WORDS_UNIQUE, $count));

--EXPECT--
bool(true)

