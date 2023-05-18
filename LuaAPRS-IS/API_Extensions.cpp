#include "API.hpp"

#include "Extensions/Types.hpp"
#include "Extensions/System.hpp"
#include "Extensions/Process.hpp"

void APRS::API::RegisterExtensions()
{
	APRS_API_RegisterGlobalFunction(is_null);

	APRS_API_RegisterGlobalFunction(system_sleep);
	APRS_API_RegisterGlobalFunction(system_get_idle_time);
	APRS_API_RegisterGlobalFunction(system_get_timestamp);

	APRS_API_RegisterGlobalFunction(process_get_root_directory);
}
