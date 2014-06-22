#ifndef __PHP_HLL_H
#define __PHP_HLL_H

#define PHP_HLL_EXTNAME "hll"
#define PHP_HLL_EXTVER  "1.0"
#define PHP_HLL_DESCRIPTOR_RES_NAME "HyperLogLog"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

extern zend_module_entry hll_module_entry;
#define phpext_hll_ptr &hll_module_entry;

#endif
