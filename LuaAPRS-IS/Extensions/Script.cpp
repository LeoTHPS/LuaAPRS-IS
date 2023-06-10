#include "API.hpp"
#include "Script.hpp"

#include <AL/Game/Loop.hpp>

AL::int16 script_exit_code = SCRIPT_EXIT_CODE_SUCCESS;

void                 script_init()
{
	script_exit_code = 0;
}
void                 script_deinit()
{
}

AL::int16            script_get_exit_code()
{
	return script_exit_code;
}
void                 script_set_exit_code(AL::int16 value)
{
	script_exit_code = value;
}

APRS::IS::Extension* script_load_extension(const char* path)
{
	if (auto extension = APRS::IS::API::LoadExtension(path))
		return extension;

	return nullptr;
}
void                 script_unload_extension(APRS::IS::Extension* extension)
{
	if (extension != nullptr)
		APRS::IS::API::UnloadExtension(extension);
}

void                 script_enter_loop(AL::uint32 tick_rate, script_loop_on_update_callback callback)
{
	AL::Game::Loop::Run(
		tick_rate,
		[&callback](AL::TimeSpan _delta)
		{
			return callback(_delta.ToMilliseconds());
		}
	);
}
