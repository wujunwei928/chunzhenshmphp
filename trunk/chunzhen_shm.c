/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 226204 2007-01-01 19:32:10Z iliaa $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_chunzhen_shm.h"

#include "chunzhen.h"

/* If you declare any globals in php_chunzhen_shm.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(chunzhen_shm)
*/

/* True global resources - no need for thread safety here */
static int le_chunzhen_shm;

static ChunZhenDB*	g_pDBTemplate;

ZEND_BEGIN_ARG_INFO(chunzhen_getIPLocation_arginfo, 0)
	ZEND_ARG_INFO(0, ipstr)
ZEND_END_ARG_INFO()

/* {{{ chunzhen_shm_functions[]
 *
 * Every user visible function must have an entry in chunzhen_shm_functions[].
 */
zend_function_entry chunzhen_shm_functions[] = {
	PHP_FE(chunzhen_getIPLocation, chunzhen_getIPLocation_arginfo)
	PHP_FE(confirm_chunzhen_shm_compiled,	NULL)		/* For testing, remove later. */
	{NULL, NULL, NULL}	/* Must be the last line in chunzhen_shm_functions[] */
};
/* }}} */

/* {{{ chunzhen_shm_module_entry
 */
zend_module_entry chunzhen_shm_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"chunzhen_shm",
	chunzhen_shm_functions,
	PHP_MINIT(chunzhen_shm),
	PHP_MSHUTDOWN(chunzhen_shm),
	PHP_RINIT(chunzhen_shm),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(chunzhen_shm),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(chunzhen_shm),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CHUNZHEN_SHM
ZEND_GET_MODULE(chunzhen_shm)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini*/
PHP_INI_BEGIN()
    PHP_INI_ENTRY("chunzhen_shm.chunzhen_db", "QQWry.Dat", PHP_INI_SYSTEM, NULL)
PHP_INI_END()
/**/
/* }}} */

/* {{{ php_chunzhen_shm_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_chunzhen_shm_init_globals(zend_chunzhen_shm_globals *chunzhen_shm_globals)
{
	chunzhen_shm_globals->global_value = 0;
	chunzhen_shm_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(chunzhen_shm)
{
	/* If you have INI entries, uncomment these lines  */
	REGISTER_INI_ENTRIES();
	/**/
	g_pDBTemplate	= new ChunZhenDB(INI_STR("chunzhen_shm.chunzhen_db"));
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(chunzhen_shm)
{
	/* uncomment this line if you have INI entries */
	UNREGISTER_INI_ENTRIES();
	/**/
	delete g_pDBTemplate;
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(chunzhen_shm)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(chunzhen_shm)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(chunzhen_shm)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "chunzhen_shm support", "enabled");
	php_info_print_table_row(2, "chunzhen_shm db", INI_STR("chunzhen_shm.chunzhen_db"));
	char	*szTempPtr;
	spprintf(&szTempPtr, 0, "%d", g_pDBTemplate->GetDBSize());
	php_info_print_table_row(2, "chunzhen db size", szTempPtr);
	efree(szTempPtr);
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

PHP_FUNCTION(chunzhen_getIPLocation)
{
	char	*szIPStr;
	int		iIPLen;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &szIPStr, &iIPLen) == FAILURE)
	{
		RETURN_FALSE;
	}

#if ZTS
	unsigned char szChunZhenDBMem[sizeof(ChunZhenDB)];
	memcpy(szChunZhenDBMem, g_pDBTemplate, sizeof(ChunZhenDB));
	ChunZhenDB*	stDB	= (ChunZhenDB*)	szChunZhenDBMem;
#else
	ChunZhenDB*	stDB	=  g_pDBTemplate;
#endif
	IPEntry		stIPEntry;
	memset(&stIPEntry, '\0', sizeof(stIPEntry));
	bool bTempRet = stDB->GetLocation(szIPStr, iIPLen, stIPEntry);
	if(bTempRet)
	{
		array_init(return_value);
		char	szBuffer[64];

		memset(szBuffer, '\0', sizeof(szBuffer));
		IPNumtoStr(szBuffer, sizeof(szBuffer) - 1, stIPEntry.nBeginIP);
		add_assoc_string(return_value, "BeginIP", szBuffer, 1);

		memset(szBuffer, '\0', sizeof(szBuffer));
		IPNumtoStr(szBuffer, sizeof(szBuffer) - 1, stIPEntry.nEndIP);
		add_assoc_string(return_value, "EndIP", szBuffer, 1);

		add_assoc_string(return_value, "Country", (char*) stIPEntry.szCountry, 1);
		add_assoc_string(return_value, "Area", (char*) stIPEntry.szArea, 1);
		return;
	}
	RETURN_FALSE;
}


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_chunzhen_shm_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_chunzhen_shm_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "chunzhen_shm", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
