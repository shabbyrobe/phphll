#include "php_hll.h"
#include "../hyperloglog.h"
#include <zend_exceptions.h>
#include <zend_types.h>
#include <ext/standard/php_var.h>

#define zend_uint uint32_t

/* {{{ macros */
#define HLL_ARG_ONLY()\
    robj *hll;\
    zval *hll_resource;\
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &hll_resource) == FAILURE) {\
        RETURN_NULL(); \
        goto cleanup; \
    }\
    if ((hll = (robj *)zend_fetch_resource(Z_RES_P(hll_resource), PHP_HLL_DESCRIPTOR_RES_NAME, hll_descriptor)) == NULL) { \
        RETVAL_FALSE; \
        goto cleanup; \
    }

#define HLL_OBJ_ARG_ONLY()\
    zval *object, *hll_resource, rv; \
    robj *hll; \
    if (zend_parse_method_parameters(ZEND_NUM_ARGS(), getThis(), "O", &object, hll_hyperloglog_ce) == FAILURE) { \
        RETVAL_FALSE; \
        goto cleanup; \
    } \
    hll_resource = zend_read_property(hll_hyperloglog_ce, object, ZEND_STRL("hll"), 0, &rv); \
    if ((hll = (robj *)zend_fetch_resource(Z_RES_P(hll_resource), PHP_HLL_DESCRIPTOR_RES_NAME, hll_descriptor)) == NULL ) { \
        RETVAL_FALSE; \
        goto cleanup; \
    }

#define HLL_OBJ_GET_RESOURCE(V, O) \
    do { \
        zval rv; \
        zval *hll_resource = zend_read_property(hll_hyperloglog_ce, (O), ZEND_STRL("hll"), 0, &rv); \
        (V) = (robj *) zend_fetch_resource(Z_RES_P(hll_resource), PHP_HLL_DESCRIPTOR_RES_NAME, hll_descriptor); \
    } while(0);

#ifndef PHP_FE_END
#define PHP_FE_END {NULL, NULL, NULL}
#endif
/* }}} */

#if PHP_MAJOR_VERSION == 7
#if PHP_MINOR_VERSION < 4
static zend_always_inline zend_bool try_convert_to_string(zval *op) {
    if (Z_TYPE_P(op) == IS_STRING) {
        return 1;
    }
    convert_to_string(op);
    return 1;
}
#endif
#endif

static int hll_descriptor;

static zend_class_entry *hll_hyperloglog_ce;
static zend_class_entry *hll_hyperloglogexception_ce;

/* {{{ Helpers */
static int php_hll_create(INTERNAL_FUNCTION_PARAMETERS, robj **hll, zend_bool allow_sparse)
{
    int ret = FAILURE;

    *hll = hllCreate();
    if (*hll == NULL) {
        php_error_docref(NULL, E_ERROR, "HLL could not be created");
        RETVAL_FALSE;
        goto cleanup;
    }

    if (!allow_sparse) {
        if (hllSparseToDense(*hll) != HLL_OK) {
            hllFree(*hll);
            php_error_docref(NULL, E_ERROR, "HLL could not be converted to dense");
            RETVAL_FALSE;
            goto cleanup;
        }
    }

    ret = SUCCESS;

cleanup:
    return ret;
}

static void php_hll_descriptor_dtor(zend_resource *rsrc)
{
    robj *hll = (robj *)rsrc->ptr;
    hllFree(hll);
}

static int php_hll_serialize(zval *object, unsigned char **buffer, size_t *buf_len, zend_serialize_data *data)
{
    robj *hll = NULL;
    HLL_OBJ_GET_RESOURCE(hll, object);

    smart_str buf = {0};
    zval zv;
    php_serialize_data_t serialize_data = (php_serialize_data_t) data;

    PHP_VAR_SERIALIZE_INIT(serialize_data);

    sds raw = hllRaw(hll);
    ZVAL_STRINGL(&zv, raw, sdslen(raw));
    php_var_serialize(&buf, &zv, &serialize_data);
    sdsfree(raw);

    PHP_VAR_SERIALIZE_DESTROY(serialize_data);

    *buffer = (unsigned char *) estrndup(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
    *buf_len = ZSTR_LEN(buf.s);
    zend_string_release(buf.s);

    return SUCCESS;
}

static int php_hll_unserialize(zval *object, zend_class_entry *ce, const unsigned char *buf, size_t buf_len, zend_unserialize_data *data)
{
    // This imitates gmp_unserialize.

    int retval = SUCCESS;
    zval zv;
    sds hll_str = NULL;
    robj *hll = NULL;
    const unsigned char *p, *max;

    php_unserialize_data_t unserialize_data = (php_unserialize_data_t) data;
    PHP_VAR_UNSERIALIZE_INIT(unserialize_data);

    p = buf;
    max = buf + buf_len;

    if (!php_var_unserialize(&zv, &p, max, &unserialize_data)
        || Z_TYPE(zv) != IS_STRING
    ) {
        zend_throw_exception(NULL, "Could not unserialize HLL", 0);
        goto fail;
    }

    hll_str = sdsnewlen(Z_STRVAL(zv), Z_STRLEN(zv));
    if (hllLoad(&hll, hll_str) == FAILURE) {
        zend_throw_exception(NULL, "Could not create HLL from unserialized string", 0);
        goto fail;
    }

    object_init_ex(object, hll_hyperloglog_ce);

    zend_resource *res = zend_register_resource(hll, hll_descriptor);
    add_property_resource(object, "hll", res);

    goto cleanup;
fail:
    retval = FAILURE;
cleanup:
    PHP_VAR_UNSERIALIZE_DESTROY(unserialize_data);
    sdsfree(hll_str);
    return retval;
}

static int php_hll_from_zval(zval *in, robj **out)
{
    int ret = SUCCESS;

    if (Z_TYPE_P(in) == IS_RESOURCE) {
        if ((*out = (robj *) zend_fetch_resource(Z_RES_P(in), PHP_HLL_DESCRIPTOR_RES_NAME, hll_descriptor)) == NULL) {
            goto fail;
        }
    }
    else if (Z_TYPE_P(in) == IS_OBJECT) { 
        if (!instanceof_function(Z_OBJCE_P(in), hll_hyperloglog_ce)) {
            php_error_docref(NULL, E_WARNING, "Supplied argument is not a valid HyperLogLog class");
            goto fail;
        }

        zval rv;
        zval *hll_resource = zend_read_property(hll_hyperloglog_ce, in, ZEND_STRL("hll"), 0, &rv);
        if ((*out = (robj *)zend_fetch_resource(Z_RES_P(hll_resource), PHP_HLL_DESCRIPTOR_RES_NAME, hll_descriptor)) == NULL) {
            goto fail;
        }
    }
    else {
        php_error_docref(NULL, E_WARNING, "Supplied argument is not a valid HyperLogLog resource or HyperLogLog class");
        goto fail;
    }

    if (*out == NULL) {
        php_error_docref(NULL, E_WARNING, "Could not fetch HyperLogLog"); 
        goto fail;
    }

    goto cleanup;
fail:
    ret = FAILURE;
cleanup:
    return ret;
}

static int php_hll_array_to_list(INTERNAL_FUNCTION_PARAMETERS, HashTable *in, hll **out)
{
    int ret = FAILURE;

    int i = 0;
    hll *sources = *out;

    zval *current;
    
    ZEND_HASH_FOREACH_VAL(in, current) {
        if (php_hll_from_zval(current, &sources[i++]) == FAILURE) {
            goto fail;
        }
    } ZEND_HASH_FOREACH_END();

    ret = SUCCESS;
    goto cleanup;

fail:
    RETVAL_FALSE;
cleanup:
    return ret;
}

static int php_hll_args_to_list(INTERNAL_FUNCTION_PARAMETERS, zval *args, int sources_len, hll **sources_in)
{
    int ret = FAILURE;

    int i = 0;
    hll *sources = NULL;

    sources = *sources_in;

    for (i = 0; i < sources_len; i++) {
        if (php_hll_from_zval(&args[i], &sources[i]) == FAILURE) {
            goto fail;
        }
    }
    ret = SUCCESS;
    goto cleanup;

fail:
    RETVAL_FALSE;
cleanup:
    return ret;
}

static int php_hll_info(INTERNAL_FUNCTION_PARAMETERS, robj *hll)
{
    array_init(return_value);

    uint8_t type = ((struct hllhdr *)hll->ptr)->encoding;
    if (type == HLL_DENSE)
        add_assoc_string(return_value, "encoding", "dense");
    else if (type == HLL_SPARSE)
        add_assoc_string(return_value, "encoding", "sparse");
    else 
        add_assoc_string(return_value, "encoding", "unknown");

    return SUCCESS;
}

static int php_hll_add(INTERNAL_FUNCTION_PARAMETERS, robj *hll, zval *data, int *updated)
{
    int ret = FAILURE;

    char **add_strings = NULL;
    int add_idx = 0;
    int i = 0;

    switch (Z_TYPE_P(data)) {
    case IS_NULL:
    case IS_TRUE:
    case IS_FALSE:
    case IS_LONG:
    case IS_DOUBLE:
    case IS_STRING:
    case IS_OBJECT: ;
        zval duplicate = *data;
        zval_copy_ctor(&duplicate);
        if (!try_convert_to_string(&duplicate)) {
            zval_dtor(&duplicate);
            goto cleanup;
        }
        {
            sds input = sdsnew(Z_STRVAL(duplicate));
            *updated = pfAdd(hll, input);
            sdsfree(input);
        }
        zval_dtor(&duplicate);

        if (*updated == HLL_ERR) {
            php_error_docref(NULL, E_WARNING, "Could not add element to HyperLogLog");
        }
        ret = SUCCESS;
    break;

    case IS_ARRAY: ;
        int cnt = zend_hash_num_elements(Z_ARRVAL_P(data));
        zval *current;
        add_strings = ecalloc(cnt + 1, sizeof(char *));

        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(data), current) {
            switch (Z_TYPE_P(current)) {
                case IS_NULL:
                case IS_TRUE:
                case IS_FALSE:
                case IS_LONG:
                case IS_DOUBLE:
                case IS_STRING:
                case IS_OBJECT: ;
                    zval duplicate = *current; 
                    zval_copy_ctor(&duplicate);
                    if (!try_convert_to_string(&duplicate)) {
                        zval_dtor(&duplicate);
                        goto cleanup;
                    }
                    add_strings[add_idx++] = sdsnew(Z_STRVAL(duplicate));
                    zval_dtor(&duplicate);
                break;

                default:
                    php_error_docref(NULL, E_WARNING, "Invalid type");
                    goto cleanup;
            }

        } ZEND_HASH_FOREACH_END();

        *updated = pfAddMany(hll, add_strings, cnt);

        if (*updated == HLL_ERR) {
            php_error_docref(NULL, E_WARNING, "Could not add element to HyperLogLog");
        }
        ret = SUCCESS;

    break;

    default:
        php_error_docref(NULL, E_WARNING, "Argument could not be converted to string");
    }

cleanup:
    if (add_strings) {
        for (i = 0; i < add_idx; i++) {
            sdsfree(add_strings[i]);
        }
        efree(add_strings);
    }
    
    return ret;
}

static int php_hll_load(INTERNAL_FUNCTION_PARAMETERS, robj **hll, char *input, size_t input_len)
{
    int ret = FAILURE;

    sds input_sds = sdsnewlen(input, input_len);
    int res = hllLoad(hll, input_sds);
    if (*hll == NULL || res != HLL_OK) {
        if (*hll != NULL) hllFree(*hll);
        php_error_docref(NULL, E_WARNING, "Supplied HLL dump was invalid");
        RETVAL_FALSE;
        goto cleanup;
    }

    ret = SUCCESS;

cleanup:
    if (input_sds != NULL) sdsfree(input_sds);
    return ret;
}
/* }}} */

/* {{{ proto void HyperLogLog::__construct([bool allowSparse = false])
 * proto void HyperLogLog::__construct(string hllDump)
 * proto void HyperLogLog::__construct(array hllsToMerge)
 */
PHP_METHOD(HyperLogLog, __construct)
{
    zval *object = NULL;
    zval *input = NULL;
    robj *hll_obj = NULL;
    hll *hll_sources = NULL;
    int sources_len = 0;

    zend_error_handling error_handling;
    zend_replace_error_handling(EH_THROW, hll_hyperloglogexception_ce, &error_handling);

    if (zend_parse_method_parameters(ZEND_NUM_ARGS(), getThis(), "O|z", 
                &object, hll_hyperloglog_ce, &input) == FAILURE) {
        RETVAL_FALSE;
        goto cleanup;
    }

    int is_null = input == NULL || Z_TYPE_P(input) == IS_NULL;
    if (is_null || Z_TYPE_P(input) == IS_TRUE || Z_TYPE_P(input) == IS_FALSE) {
        zend_bool allow_sparse = is_null ? 0 : Z_TYPE_P(input) == IS_TRUE;
        if (php_hll_create(INTERNAL_FUNCTION_PARAM_PASSTHRU, &hll_obj, allow_sparse) != SUCCESS) {
            goto fail;
        }
    }
    else if (Z_TYPE_P(input) == IS_STRING) {
        if (php_hll_load(INTERNAL_FUNCTION_PARAM_PASSTHRU, &hll_obj, Z_STRVAL_P(input), Z_STRLEN_P(input)) != SUCCESS) {
            goto fail;
        }
    }
    else if (Z_TYPE_P(input) == IS_ARRAY) {
        HashTable *hll_ht = Z_ARRVAL_P(input);
        sources_len = zend_hash_num_elements(hll_ht);
        hll_sources = emalloc(sizeof(hll) * sources_len);
        if (php_hll_array_to_list(INTERNAL_FUNCTION_PARAM_PASSTHRU, hll_ht, &hll_sources) != SUCCESS) {
            goto fail;
        }
        if (php_hll_create(INTERNAL_FUNCTION_PARAM_PASSTHRU, &hll_obj, 0) != SUCCESS) {
            goto fail;
        }
        if (pfMerge(hll_obj, hll_sources, sources_len) != HLL_OK) {
            php_error_docref(NULL, E_WARNING, "Expected array or resource");
            goto fail;
        }
    }

    zend_resource *res = zend_register_resource(hll_obj, hll_descriptor);
    add_property_resource(object, "hll", res);
    goto cleanup;

fail:
    RETVAL_FALSE;
cleanup:
    zend_restore_error_handling(&error_handling);
    efree(hll_sources);
    return;
}
/* }}} */

/* {{{ proto resource hll_create([bool allowSparse = false]) */
PHP_FUNCTION(hll_create)
{
    robj *hll;
    zend_bool allow_sparse = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &allow_sparse) == FAILURE) {
        RETVAL_FALSE;
        goto cleanup;
    }

    if (php_hll_create(INTERNAL_FUNCTION_PARAM_PASSTHRU, &hll, allow_sparse) != SUCCESS) {
        RETVAL_FALSE;
        goto cleanup;
    }

    RETURN_RES(zend_register_resource(hll, hll_descriptor));

cleanup:
    return;
}
/* }}} */

/* {{{ proto HyperLogLog HyperLogLog::merge ( mixed $hyperLogLog [ , mixed $... ])
 * proto HyperLogLog HyperLogLog::merge ( array $hyperLogLogs )
 */
PHP_METHOD(HyperLogLog, merge)
{
    int argc = 0;
    zval *args = NULL;
    hll *hll_sources = NULL;
    int sources_len = 0;
    zval *object, *hll_resource, rv;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS(), getThis(), "O+", &object, hll_hyperloglog_ce, &args, &argc) == FAILURE) {
        goto cleanup;
    }

    robj *hll_target = NULL;

    hll_resource = zend_read_property(hll_hyperloglog_ce, object, ZEND_STRL("hll"), 0, &rv);

    if ((hll_target = (robj *)zend_fetch_resource(Z_RES_P(hll_resource), PHP_HLL_DESCRIPTOR_RES_NAME, hll_descriptor)) == NULL) {
        RETURN_FALSE;
    }

    if (hll_target == NULL) {
        php_error_docref(NULL, E_ERROR, "HLL could not be retrieved");
        goto fail;
    }

    if (argc == 1 && Z_TYPE_P(args) == IS_ARRAY) {
        HashTable *hll_ht = Z_ARRVAL(args[0]);
        sources_len = zend_hash_num_elements(hll_ht) + 1;
        hll_sources = emalloc(sizeof(hll) * sources_len);
        if (php_hll_array_to_list(INTERNAL_FUNCTION_PARAM_PASSTHRU, hll_ht, &hll_sources) != SUCCESS) {
            goto fail;
        }
    }
    else {
        sources_len = argc + 1;
        hll_sources = emalloc(sizeof(hll) * sources_len);
        if (php_hll_args_to_list(INTERNAL_FUNCTION_PARAM_PASSTHRU, args, argc, &hll_sources) != SUCCESS) {
            goto fail;
        }
    }

    if (sources_len < 2) {
        zend_wrong_param_count();
        goto fail;
    }

    hll_sources[sources_len - 1] = hll_target;

    if (pfMerge(hll_target, hll_sources, sources_len) != HLL_OK) {
        php_error_docref(NULL, E_WARNING, "Expected array or resource");
        goto fail;
    }

    RETVAL_ZVAL(getThis(), 1, 0);
    goto cleanup;

fail:
    RETVAL_FALSE;
    
cleanup:
    efree(hll_sources);
    return;
}
/* }}} */

/* {{{ proto resource hll_merge( mixed $hyperLogLog1 , mixed $hyperLogLog2 [ , mixed $... ])
 * proto resource hll_merge( array $hyperLogLogs )
 */
PHP_FUNCTION(hll_merge)
{
    int argc = 0;
    zval *args = NULL;
    hll *hll_sources = NULL;
    int sources_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc) == FAILURE) {
        goto cleanup;
    }
    
    robj *hll_target = NULL;
    hll_target = hllCreate();
    if (hll_target == NULL) {
        php_error_docref(NULL, E_WARNING, "HLL could not be created");
        goto fail;
    }

    if (argc == 1 && Z_TYPE_P(args) == IS_ARRAY) {
        HashTable *hll_ht = Z_ARRVAL(args[0]);
        sources_len = zend_hash_num_elements(hll_ht);
        hll_sources = emalloc(sizeof(hll) * sources_len);
        if (php_hll_array_to_list(INTERNAL_FUNCTION_PARAM_PASSTHRU, hll_ht, &hll_sources) != SUCCESS) {
            goto fail;
        }
    }
    else {
        sources_len = argc;
        hll_sources = emalloc(sizeof(hll) * argc);
        if (php_hll_args_to_list(INTERNAL_FUNCTION_PARAM_PASSTHRU, args, argc, &hll_sources) != SUCCESS) {
            goto fail;
        }
    }

    if (sources_len < 2) {
        zend_wrong_param_count();
        goto fail;
    }

    if (pfMerge(hll_target, hll_sources, sources_len) != HLL_OK) {
        php_error_docref(NULL, E_WARNING, "Expected array or resource");
        goto fail;
    }

    RETURN_RES(zend_register_resource(hll_target, hll_descriptor));
    goto cleanup;

fail:
    RETVAL_FALSE;
    
cleanup:
    efree(hll_sources);
    return;
}
/* }}} */

/* {{{ proto HyperLogLog HyperLogLog::add( scalar $value , [ bool &$updated ])
 * proto HyperLogLog HyperLogLog::add( array $values , [ bool &$updated ])
 */
ZEND_BEGIN_ARG_INFO_EX(ai_HyperLogLog_add, 0, 0, 1)
    ZEND_ARG_INFO(0, input)
    ZEND_ARG_INFO(1, updated)
ZEND_END_ARG_INFO()
PHP_METHOD(HyperLogLog, add)
{
    zval *object, *data, rv;
    robj *hll;
    int updated = 0;
    zend_error_handling errorh;
    zval *upd_arg = NULL;

    zend_replace_error_handling(EH_THROW, hll_hyperloglogexception_ce, &errorh);
    if (zend_parse_method_parameters(ZEND_NUM_ARGS(), getThis(), 
            "Oz|z/", &object, hll_hyperloglog_ce, &data, &upd_arg) == FAILURE) {
        goto fail;
    }

    zval *hll_resource = zend_read_property(hll_hyperloglog_ce, object, ZEND_STRL("hll"), 0, &rv);

    if ((hll = (robj *)zend_fetch_resource(Z_RES_P(hll_resource), PHP_HLL_DESCRIPTOR_RES_NAME, hll_descriptor)) == NULL) {
        goto fail;
    }

    if (php_hll_add(INTERNAL_FUNCTION_PARAM_PASSTHRU, hll, data, &updated) != SUCCESS) {
        goto fail;
    }

    if (upd_arg) {
        zval_ptr_dtor(upd_arg);
        ZVAL_BOOL(upd_arg, updated);
    }

    RETVAL_ZVAL(getThis(), 1, 0);
    goto cleanup;
fail:
    RETVAL_FALSE;
cleanup:
    zend_restore_error_handling(&errorh);
}
/* }}} */

/* {{{ proto bool hll_add ( resource $hll , scalar $value )
 * proto bool hll_add ( resource $hll , array $values )
 */
PHP_FUNCTION(hll_add)
{
    robj *hll;
    zval *hll_resource;
    zval *data;
    int updated = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rz", &hll_resource, &data) == FAILURE) {
        RETURN_NULL();
    }
    if ((hll = (robj *)zend_fetch_resource(Z_RES_P(hll_resource), PHP_HLL_DESCRIPTOR_RES_NAME, hll_descriptor)) == NULL ) {
        goto cleanup;
    }

    if (php_hll_add(INTERNAL_FUNCTION_PARAM_PASSTHRU, hll, data, &updated) != SUCCESS) {
        RETVAL_FALSE;
        goto cleanup;
    }

cleanup:
    RETURN_BOOL(updated == 1);
}
/* }}} */

/* {{{ proto int HyperLogLog::count( void ) */
PHP_METHOD(HyperLogLog, count)
{
    HLL_OBJ_ARG_ONLY();
    uint64_t count = pfCount(hll);
    RETVAL_LONG(count);
fail:
cleanup:
    return;
}
/* }}} */

/* {{{ proto int hll_count ( mixed $hll [ , mixed $... ]) */
PHP_FUNCTION(hll_count)
{
    hll *hll_sources = NULL;
    zval *args = NULL;
    int argc = 0;
    int i = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc) == FAILURE) {
        goto cleanup;
    }

    hll_sources = emalloc(sizeof(hll) * argc);
    if (php_hll_args_to_list(INTERNAL_FUNCTION_PARAM_PASSTHRU, args, argc, &hll_sources) != SUCCESS) {
        RETVAL_FALSE;
        goto cleanup;
    }

    if (argc == 1) {
        uint64_t count = pfCount(hll_sources[0]);
        RETVAL_LONG(count);
    }
    else if (argc > 1) {
        uint64_t count = pfCountMerged(hll_sources, argc);
        RETVAL_LONG(count);
    }
    else {
        zend_wrong_param_count();
    }

cleanup:
    efree(hll_sources);
}
/* }}} */

/* {{{ proto HyperLogLog HyperLogLog::promote ( void ) */
PHP_METHOD(HyperLogLog, promote)
{
    HLL_OBJ_ARG_ONLY();

    if (hllSparseToDense(hll) != HLL_OK) {
        php_error_docref(NULL, E_WARNING, "HLL could not be promoted");
    }
    RETVAL_ZVAL(getThis(), 1, 0);
cleanup:
    return;
}
/* }}} */

/* {{{ proto void hll_promote ( resource $hll ) */
PHP_FUNCTION(hll_promote)
{
    HLL_ARG_ONLY();
    if (hllSparseToDense(hll) != HLL_OK) {
        php_error_docref(NULL, E_WARNING, "HLL could not be promoted");
    }

cleanup:
    RETURN_NULL();
}
/* }}} */

/* {{{ proto array HyperLogLog::info ( void ) */
PHP_METHOD(HyperLogLog, info)
{
    HLL_OBJ_ARG_ONLY();
    
    if (php_hll_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, hll) != SUCCESS) {
        php_error_docref(NULL, E_WARNING, "Could not fetch info");
        RETVAL_FALSE;
        goto cleanup;
    }

cleanup:
    return;
}
/* }}} */

/* {{{ proto array hll_info ( resource $hll ) */
PHP_FUNCTION(hll_info)
{
    HLL_ARG_ONLY();

    if (php_hll_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, hll) != SUCCESS) {
        php_error_docref(NULL, E_WARNING, "Could not fetch info");
        RETVAL_FALSE;
        goto cleanup;
    }

cleanup:
    return;
}
/* }}} */

/* {{{ proto string HyperLogLog::dump ( void ) */
PHP_METHOD(HyperLogLog, dump)
{
    HLL_OBJ_ARG_ONLY();

    sds raw = hllRaw(hll);
    ZVAL_STRINGL(return_value, raw, sdslen(raw));
    sdsfree(raw);

cleanup:
    return;
}
/* }}} */

/* {{{ proto string hll_dump ( resource $hll ) */
PHP_FUNCTION(hll_dump)
{
    HLL_ARG_ONLY();
    
    sds raw = hllRaw(hll);
    ZVAL_STRINGL(return_value, raw, sdslen(raw));
    sdsfree(raw);

cleanup:
    return;
}
/* }}} */

/* {{{ proto resource hll_load ( string $hllDump ) */
PHP_FUNCTION(hll_load)
{
    char *input;
    size_t input_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &input, &input_len) == FAILURE) {
        RETURN_NULL();
    }

    robj *hll = NULL;
    if (php_hll_load(INTERNAL_FUNCTION_PARAM_PASSTHRU, &hll, input, input_len) != SUCCESS) {
        RETVAL_FALSE;
        goto cleanup;
    }

    RETURN_RES(zend_register_resource(hll, hll_descriptor));

cleanup:
    return;
}
/* }}} */

const static zend_function_entry hll_hyperloglog_methods[] = {
    PHP_ME(HyperLogLog, __construct, NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    PHP_ME(HyperLogLog, merge, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(HyperLogLog, add, ai_HyperLogLog_add, ZEND_ACC_PUBLIC)
    PHP_ME(HyperLogLog, count, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(HyperLogLog, promote, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(HyperLogLog, info, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(HyperLogLog, dump, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

const static zend_function_entry hll_hyperloglogexception_methods[] = {
    PHP_FE_END
};

static zend_function_entry php_hll_functions[] = {
    PHP_FE(hll_create, NULL)
    PHP_FE(hll_add, NULL)
    PHP_FE(hll_merge, NULL)
    PHP_FE(hll_count, NULL)
    PHP_FE(hll_promote, NULL)
    PHP_FE(hll_info, NULL)
    PHP_FE(hll_dump, NULL)
    PHP_FE(hll_load, NULL)
    PHP_FE_END
};

zend_module_entry hll_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_HLL_EXTNAME,
    php_hll_functions,
    PHP_MINIT(hll),    /* MINIT */
    NULL,    /* MSHUTDOWN */
    NULL,    /* RINIT */
    NULL,    /* RSHUTDOWN */
    NULL,    /* MINFO */
    PHP_HLL_EXTVER,
    STANDARD_MODULE_PROPERTIES,
};

PHP_MINIT_FUNCTION(hll)
{
    zend_class_entry ce = {0};

    hll_descriptor = zend_register_list_destructors_ex(
        php_hll_descriptor_dtor, NULL, PHP_HLL_DESCRIPTOR_RES_NAME,
        module_number);
    
    hll_ce: { 
        INIT_CLASS_ENTRY(ce, "HyperLogLog", hll_hyperloglog_methods);
        hll_hyperloglog_ce = zend_register_internal_class(&ce);
        hll_hyperloglog_ce->serialize = php_hll_serialize;
        hll_hyperloglog_ce->unserialize = php_hll_unserialize;
        hll_hyperloglog_ce->clone = NULL;

        zend_declare_property_null(hll_hyperloglog_ce, ZEND_STRL("hll"), ZEND_ACC_PUBLIC);
    }

    hll_exception: {
        INIT_CLASS_ENTRY(ce, "HyperLogLogException", hll_hyperloglogexception_methods);
        hll_hyperloglogexception_ce = zend_register_internal_class_ex(&ce, zend_exception_get_default());
        hll_hyperloglogexception_ce->ce_flags |= ZEND_ACC_FINAL;
    }

    return SUCCESS;
}

#ifdef COMPILE_DL_HLL
ZEND_GET_MODULE(hll)
#endif
