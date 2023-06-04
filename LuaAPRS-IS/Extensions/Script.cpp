#include "API.hpp"
#include "Script.hpp"

AL::int16 script_exit_code = SCRIPT_EXIT_CODE_SUCCESS;

void                script_init()
{
	script_exit_code = 0;
}
void                script_deinit()
{
}

AL::int16           script_get_exit_code()
{
	return script_exit_code;
}
void                script_set_exit_code(AL::int16 value)
{
	script_exit_code = value;
}

APRS_IS::Extension* script_load_extension(const char* path)
{
	if (auto extension = APRS_IS::API::LoadExtension(path))
		return extension;

	return nullptr;
}
void                script_unload_extension(APRS_IS::Extension* extension)
{
	if (extension != nullptr)
		APRS_IS::API::UnloadExtension(extension);
}