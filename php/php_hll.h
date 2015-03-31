#ifndef __PHP_HLL_H
#define __PHP_HLL_H

#include "php.h"

#define PHP_HLL_EXTNAME "hll"
#define PHP_HLL_EXTVER  "1.0"
#define PHP_HLL_DESCRIPTOR_RES_NAME "HyperLogLog"

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern zend_module_entry hll_module_entry;

#define phpext_hll_ptr &hll_module_entry;

PHP_MINIT_FUNCTION(hll);

PHP_METHOD(HyperLogLog, __construct);
PHP_METHOD(HyperLogLog, merge);
PHP_METHOD(HyperLogLog, add);
PHP_METHOD(HyperLogLog, count);
PHP_METHOD(HyperLogLog, promote);
PHP_METHOD(HyperLogLog, info);
PHP_METHOD(HyperLogLog, dump);

PHP_FUNCTION(hll_create);
PHP_FUNCTION(hll_merge);
PHP_FUNCTION(hll_add);
PHP_FUNCTION(hll_count);
PHP_FUNCTION(hll_promote);
PHP_FUNCTION(hll_info);
PHP_FUNCTION(hll_dump);
PHP_FUNCTION(hll_load);

#endif
