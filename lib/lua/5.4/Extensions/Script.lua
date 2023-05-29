Script =
{
	GetExitCode = function()
		return script_get_exit_code();
	end,

	SetExitCode = function(value)
		script_set_exit_code(tonumber(value));
	end,

	ExitCodes =
	{
		Success = SCRIPT_EXIT_CODE_SUCCESS,

		APRS =
		{
			IS =
			{
				ConnectionFailed = SCRIPT_EXIT_CODE_APRS_IS_CONNECTION_FAILED
			}
		}
	}
};
