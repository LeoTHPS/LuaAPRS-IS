#pragma once
#include <AL/Common.hpp>

namespace APRS::IS
{
	struct Extension;
};

enum SCRIPT_EXIT_CODES : AL::int16
{
	SCRIPT_EXIT_CODE_SUCCESS,
	SCRIPT_EXIT_CODE_APRS_IS_CONNECTION_FAILED,
	SCRIPT_EXIT_CODE_SQLITE3_OPEN_FAILED,
	SCRIPT_EXIT_CODE_,
	SCRIPT_EXIT_CODE_APRS_,
	SCRIPT_EXIT_CODE_APRS_IS_,
	SCRIPT_EXIT_CODE_SQLITE3_,
};

void                 script_init();
void                 script_deinit();

AL::int16            script_get_exit_code();
void                 script_set_exit_code(AL::int16 value);

APRS::IS::Extension* script_load_extension(const char* path);
void                 script_unload_extension(APRS::IS::Extension* extension);
