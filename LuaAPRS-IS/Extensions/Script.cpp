#include "API.hpp"
#include "Script.hpp"

AL::int16 script_exit_code = SCRIPT_EXIT_CODE_SUCCESS;

void      script_init()
{
	script_exit_code = 0;
}
void      script_deinit()
{
}

AL::int16 script_get_exit_code()
{
	return script_exit_code;
}

void      script_set_exit_code(AL::int16 value)
{
	script_exit_code = value;
}
