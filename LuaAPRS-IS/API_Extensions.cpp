#include "API.hpp"

#include "Extensions/Types.hpp"
#include "Extensions/Mutex.hpp"
#include "Extensions/Script.hpp"
#include "Extensions/System.hpp"
#include "Extensions/Thread.hpp"
#include "Extensions/Process.hpp"
#include "Extensions/SQLite3.hpp"

#define APRS_IS_API_DEFINE_Extension(__extension_name__, ...) void RegisterExtension_##__extension_name__(AL::Lua54::State& lua) __VA_ARGS__
#define APRS_IS_API_RegisterExtension(__extension_name__)     RegisterExtension_##__extension_name__(lua)

APRS_IS_API_DEFINE_Extension(Types,
{
	APRS_IS_API_RegisterGlobalFunction(is_null);
})

APRS_IS_API_DEFINE_Extension(Mutex,
{
	APRS_IS_API_RegisterGlobalFunction(mutex_get_default_instance);
	APRS_IS_API_RegisterGlobalFunction(mutex_init);
	APRS_IS_API_RegisterGlobalFunction(mutex_deinit);
	APRS_IS_API_RegisterGlobalFunction(mutex_lock);
	APRS_IS_API_RegisterGlobalFunction(mutex_unlock);
})

APRS_IS_API_DEFINE_Extension(Script,
{
	APRS_IS_API_RegisterGlobal(SCRIPT_EXIT_CODE_SUCCESS);
	APRS_IS_API_RegisterGlobal(SCRIPT_EXIT_CODE_APRS_IS_CONNECTION_FAILED);
	APRS_IS_API_RegisterGlobal(SCRIPT_EXIT_CODE_SQLITE3_OPEN_FAILED);

	APRS_IS_API_RegisterGlobalFunction(script_get_exit_code);
	APRS_IS_API_RegisterGlobalFunction(script_set_exit_code);
})

APRS_IS_API_DEFINE_Extension(System,
{
	APRS_IS_API_RegisterGlobalFunction(system_get_idle_time);
	APRS_IS_API_RegisterGlobalFunction(system_get_timestamp);
})

APRS_IS_API_DEFINE_Extension(Thread,
{
	APRS_IS_API_RegisterGlobalFunction(thread_sleep);
	APRS_IS_API_RegisterGlobalFunction(thread_start);
	APRS_IS_API_RegisterGlobalFunction(thread_join);
})

APRS_IS_API_DEFINE_Extension(Process,
{
	APRS_IS_API_RegisterGlobalFunction(process_get_root_directory);
})

APRS_IS_API_DEFINE_Extension(SQLite3,
{
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_NONE);
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_URI);
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_CREATE);
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_READ_ONLY);
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_READ_WRITE);
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_MEMORY);
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_NO_MUTEX);
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_FULL_MUTEX);
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_NO_FOLLOW);
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_SHARED_CACHE);
	APRS_IS_API_RegisterGlobal(SQLITE3_DATABASE_FLAG_PRIVATE_CACHE);

	APRS_IS_API_RegisterGlobalFunctionEx(_sqlite3_init,                              "sqlite3_init");
	APRS_IS_API_RegisterGlobalFunctionEx(_sqlite3_deinit,                            "sqlite3_deinit");
	APRS_IS_API_RegisterGlobalFunctionEx(_sqlite3_is_open,                           "sqlite3_is_open");
	APRS_IS_API_RegisterGlobalFunctionEx(_sqlite3_open,                              "sqlite3_open");
	APRS_IS_API_RegisterGlobalFunctionEx(_sqlite3_close,                             "sqlite3_close");
	APRS_IS_API_RegisterGlobalFunctionEx(_sqlite3_execute_query,                     "sqlite3_execute_query");
	APRS_IS_API_RegisterGlobalFunctionEx(_sqlite3_execute_non_query,                 "sqlite3_execute_non_query");
	APRS_IS_API_RegisterGlobalFunctionEx(_sqlite3_query_result_row_get_column_count, "sqlite3_query_result_row_get_column_count");
	APRS_IS_API_RegisterGlobalFunctionEx(_sqlite3_query_result_row_get_column_name,  "sqlite3_query_result_row_get_column_name");
	APRS_IS_API_RegisterGlobalFunctionEx(_sqlite3_query_result_row_get_column_value, "sqlite3_query_result_row_get_column_value");
})

void APRS_IS::API::RegisterExtensions()
{
	APRS_IS_API_RegisterExtension(Types);
	APRS_IS_API_RegisterExtension(Mutex);
	APRS_IS_API_RegisterExtension(Script);
	APRS_IS_API_RegisterExtension(System);
	APRS_IS_API_RegisterExtension(Thread);
	APRS_IS_API_RegisterExtension(Process);
	APRS_IS_API_RegisterExtension(SQLite3);
}
