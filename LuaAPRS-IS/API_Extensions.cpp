#include "API.hpp"

#include "Extensions/Types.hpp"
#include "Extensions/Mutex.hpp"
#include "Extensions/Script.hpp"
#include "Extensions/System.hpp"
#include "Extensions/Thread.hpp"
#include "Extensions/Process.hpp"

void APRS_IS::API::RegisterExtensions()
{
	APRS_IS_API_RegisterGlobalFunction(is_null);

	APRS_IS_API_RegisterGlobalFunction(mutex_get_default_instance);
	APRS_IS_API_RegisterGlobalFunction(mutex_init);
	APRS_IS_API_RegisterGlobalFunction(mutex_deinit);
	APRS_IS_API_RegisterGlobalFunction(mutex_lock);
	APRS_IS_API_RegisterGlobalFunction(mutex_unlock);

	APRS_IS_API_RegisterGlobal(SCRIPT_EXIT_CODE_SUCCESS);
	APRS_IS_API_RegisterGlobal(SCRIPT_EXIT_CODE_APRS_IS_CONNECTION_FAILED);

	APRS_IS_API_RegisterGlobalFunction(script_get_exit_code);
	APRS_IS_API_RegisterGlobalFunction(script_set_exit_code);

	APRS_IS_API_RegisterGlobalFunction(system_get_idle_time);
	APRS_IS_API_RegisterGlobalFunction(system_get_timestamp);

	APRS_IS_API_RegisterGlobalFunction(thread_sleep);
	APRS_IS_API_RegisterGlobalFunction(thread_start);
	APRS_IS_API_RegisterGlobalFunction(thread_join);

	APRS_IS_API_RegisterGlobalFunction(process_get_root_directory);
}
