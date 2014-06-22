#include "php_hll.h"
#include "../hyperloglog.h"

#define HLL_ARG_ONLY()\
    robj *hll;\
    zval *hll_resource;\
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &hll_resource) == FAILURE) {\
        RETURN_NULL();\
    }\
    ZEND_FETCH_RESOURCE(hll, robj *, &hll_resource, -1, PHP_HLL_DESCRIPTOR_RES_NAME, hll_descriptor);

static int hll_descriptor;

static void php_hll_descriptor_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    robj *hll = (robj *)rsrc->ptr;
    hllFree(hll);
}

PHP_MINIT_FUNCTION(hll)
{
    hll_descriptor = zend_register_list_destructors_ex(
        php_hll_descriptor_dtor, NULL, PHP_HLL_DESCRIPTOR_RES_NAME,
        module_number);

    return SUCCESS;
}

PHP_FUNCTION(hll_create)
{
    zend_bool allowSparse = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &allowSparse) == FAILURE) {
        RETURN_FALSE;
    }

    robj *hll = hllCreate();
    if (hll == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "HLL could not be created");
        RETURN_FALSE;
    }

    if (!allowSparse) {
        if (hllSparseToDense(hll) != HLL_OK) {
            hllFree(hll);
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "HLL could not be converted to dense");
            RETURN_FALSE;
        }
    }

    ZEND_REGISTER_RESOURCE(return_value, hll, hll_descriptor);
}

PHPAPI int hll_args_to_list(INTERNAL_FUNCTION_PARAMETERS, hll **sources_in, int *sources_len)
{
    int ret = FAILURE;

    zval **args; 
    int argc = ZEND_NUM_ARGS(); 
    int i = 0;
    hll *sources = NULL;

    args = emalloc(argc * sizeof(zval*));
    if (zend_get_parameters_array(ht, argc, args) == FAILURE) {
        RETVAL_FALSE;
        goto cleanup;
    }

    if (argc == 1 && Z_TYPE_P(args[0]) == IS_ARRAY) {
        zval *data = args[0];
        *sources_len = zend_hash_num_elements(Z_ARRVAL_P(data));
        *sources_in = emalloc(sizeof(hll) * *sources_len);
        sources = *sources_in;
        
        HashPosition pos;
        zval **current;
        for (i = 0, zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(data), &pos);
                zend_hash_get_current_data_ex(Z_ARRVAL_P(data), (void **)&current, &pos) == SUCCESS;
                zend_hash_move_forward_ex(Z_ARRVAL_P(data), &pos), i++) {
            
            if (Z_TYPE_PP(current) != IS_RESOURCE) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, 
                    "Supplied argument is not a valid HyperLogLog resource"); 
                goto cleanup;
            }

            sources[i] = (hll) zend_fetch_resource(current TSRMLS_CC, -1, 
                PHP_HLL_DESCRIPTOR_RES_NAME, NULL, 1, hll_descriptor);

            if (sources[i] == NULL) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not fetch HyperLogLog"); 
                goto cleanup;
            }
        }
    }
    else {
        *sources_len = argc;
        *sources_in = emalloc(sizeof(hll) * *sources_len);
        sources = *sources_in;

        for (i = 0; i < argc; i++) {
            if (Z_TYPE_P(args[i]) != IS_RESOURCE) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, 
                    "Supplied argument is not a valid HyperLogLog resource");
                RETVAL_FALSE;
                goto cleanup;
            }
            sources[i] = (hll) zend_fetch_resource(&args[i] TSRMLS_CC, -1, 
                PHP_HLL_DESCRIPTOR_RES_NAME, NULL, 1, hll_descriptor);

            if (sources[i] == NULL) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not fetch HyperLogLog"); 
                goto cleanup;
            }
        }
    }

    ret = SUCCESS;

cleanup:
    efree(args);
    return ret;
}

PHP_FUNCTION(hll_merge)
{
    hll hll_target = NULL;
    hll *hll_sources = NULL;
    int hll_sources_len = 0;
    int i = 0;

    if (hll_args_to_list(INTERNAL_FUNCTION_PARAM_PASSTHRU, &hll_sources, &hll_sources_len) != SUCCESS) {
        RETURN_FALSE;
        goto cleanup;
    }

    if (hll_sources_len < 2) {
        zend_wrong_param_count(TSRMLS_C);
        goto cleanup;
    }

    hll_target = hllCreate();
    if (hll_target == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "HLL could not be created");
        goto cleanup;
    }

    if (pfMerge(hll_target, hll_sources, hll_sources_len) != HLL_OK) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Expected array or resource");
        goto cleanup;
    }

    ZEND_REGISTER_RESOURCE(return_value, hll_target, hll_descriptor);

cleanup:
    efree(hll_sources);
}

PHP_FUNCTION(hll_add)
{
    robj *hll;
    zval *hll_resource;
    zval *data;
    char **add_strings = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz", &hll_resource, &data) == FAILURE) {
        RETURN_NULL();
    }
    ZEND_FETCH_RESOURCE(hll, robj *, &hll_resource, -1, PHP_HLL_DESCRIPTOR_RES_NAME, hll_descriptor);

    int updated = 0;

    switch (Z_TYPE_P(data)) {
    case IS_NULL:
    case IS_BOOL:
    case IS_LONG:
    case IS_DOUBLE:
    case IS_STRING:
    case IS_OBJECT: ;
        zval duplicate = *data;
        zval_copy_ctor(&duplicate);
        convert_to_string(&duplicate);
        {
            sds input = sdsnew(Z_STRVAL(duplicate));
            updated = pfAdd(hll, input);
            sdsfree(input);
        }
        zval_dtor(&duplicate);

        if (updated == HLL_ERR) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not add element to HyperLogLog");
        }
    break;

    case IS_ARRAY: ;
        int cnt = zend_hash_num_elements(Z_ARRVAL_P(data));
        HashPosition pos;
        zval **current;
        add_strings = ecalloc(cnt + 1, sizeof(char *));
        int add_idx = 0;
        int i = 0;

        for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(data), &pos);
                zend_hash_get_current_data_ex(Z_ARRVAL_P(data), (void**)&current, &pos) == SUCCESS;
                zend_hash_move_forward_ex(Z_ARRVAL_P(data), &pos)) {

            zval duplicate = **current; 
            zval_copy_ctor(&duplicate);
            convert_to_string(&duplicate);
            add_strings[add_idx++] = sdsnew(Z_STRVAL(duplicate));
            zval_dtor(&duplicate);
        }

        updated = pfAddMany(hll, add_strings, cnt);

        for (i=0; i < cnt; i++) {
            sdsfree(add_strings[i]);
        }

        if (updated == HLL_ERR) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not add element to HyperLogLog");
        }
    break;

    default:
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid type");
    }

cleanup:
    efree(add_strings);
    RETURN_BOOL(updated == 1);
}

PHP_FUNCTION(hll_count)
{
    hll *hll_sources = NULL;
    int hll_sources_len = 0;
    int i = 0;

    if (hll_args_to_list(INTERNAL_FUNCTION_PARAM_PASSTHRU, &hll_sources, &hll_sources_len) != SUCCESS) {
        RETURN_FALSE;
        goto cleanup;
    }

    if (hll_sources_len == 1) {
        uint64_t count = pfCount(hll_sources[0]);
        RETVAL_LONG(count);
    }
    else if (hll_sources_len > 1) {
        uint64_t count = pfCountMerged(hll_sources, hll_sources_len);
        RETVAL_LONG(count);
    }
    else {
        zend_wrong_param_count(TSRMLS_C);
        goto cleanup;
    }

cleanup:
    efree(hll_sources);
}

PHP_FUNCTION(hll_promote)
{
    HLL_ARG_ONLY();
    if (hllSparseToDense(hll) != HLL_OK) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "HLL could not be promoted");
    }
    RETURN_NULL();
}

PHP_FUNCTION(hll_info)
{
    HLL_ARG_ONLY();
    array_init(return_value);

    uint8_t type = ((struct hllhdr *)hll->ptr)->encoding;
    if (type == HLL_DENSE)
        add_assoc_string(return_value, "encoding", "dense", 1);
    else if (type == HLL_SPARSE)
        add_assoc_string(return_value, "encoding", "sparse", 1);
    else 
        add_assoc_string(return_value, "encoding", "unknown", 1);
}

PHP_FUNCTION(hll_dump)
{
    HLL_ARG_ONLY();
    
    sds raw = hllRaw(hll);
    ZVAL_STRINGL(return_value, raw, sdslen(raw), 1);
    sdsfree(raw);
}

PHP_FUNCTION(hll_load)
{
    char *input;
    int input_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &input, &input_len) == FAILURE) {
        RETURN_NULL();
    }

    hll hll = NULL;
    sds input_sds = sdsnewlen(input, input_len);

    int res = hllLoad(&hll, input_sds);
    if (hll == NULL || res != HLL_OK) {
        if (hll != NULL) hllFree(hll);
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Supplied HLL dump was invalid");
        RETVAL_FALSE;
        goto cleanup;
    }

    ZEND_REGISTER_RESOURCE(return_value, hll, hll_descriptor);

cleanup:
    if (input_sds != NULL) sdsfree(input_sds);
    return;
}

static zend_function_entry php_hll_functions[] = {
    PHP_FE(hll_create, NULL)
    PHP_FE(hll_add, NULL)
    PHP_FE(hll_merge, NULL)
    PHP_FE(hll_count, NULL)
    PHP_FE(hll_promote, NULL)
    PHP_FE(hll_info, NULL)
    PHP_FE(hll_dump, NULL)
    PHP_FE(hll_load, NULL)
    { NULL, NULL, NULL }
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

#ifdef COMPILE_DL_HLL
ZEND_GET_MODULE(hll)
#endif
