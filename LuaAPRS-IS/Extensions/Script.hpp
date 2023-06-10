#pragma once
#include <AL/Common.hpp>

#include "Extension.hpp"

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

// @return false to exit loop
typedef AL::Lua54::Function::LuaCallback<bool(AL::uint64 delta_ms)> script_loop_on_update_callback;

void                 script_init();
void                 script_deinit();

AL::int16            script_get_exit_code();
void                 script_set_exit_code(AL::int16 value);

APRS::IS::Extension* script_load_extension(const char* path);
void                 script_unload_extension(APRS::IS::Extension* extension);

void                 script_enter_loop(AL::uint32 tick_rate, script_loop_on_update_callback callback);
