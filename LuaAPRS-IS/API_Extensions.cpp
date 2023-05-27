#include "API.hpp"

#include "Extensions/Types.hpp"
#include "Extensions/Mutex.hpp"
#include "Extensions/Thread.hpp"
#include "Extensions/System.hpp"
#include "Extensions/Process.hpp"

void APRS::API::RegisterExtensions()
{
	APRS_API_RegisterGlobalFunction(is_null);

	APRS_API_RegisterGlobalFunction(mutex_get_default_instance);
	APRS_API_RegisterGlobalFunction(mutex_init);
	APRS_API_RegisterGlobalFunction(mutex_deinit);
	APRS_API_RegisterGlobalFunction(mutex_lock);
	APRS_API_RegisterGlobalFunction(mutex_unlock);

	APRS_API_RegisterGlobalFunction(thread_sleep);
	APRS_API_RegisterGlobalFunction(thread_start);
	APRS_API_RegisterGlobalFunction(thread_join);

	APRS_API_RegisterGlobalFunction(system_get_idle_time);
	APRS_API_RegisterGlobalFunction(system_get_timestamp);

	APRS_API_RegisterGlobalFunction(process_get_root_directory);
}
