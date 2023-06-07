Script =
{
	LoadExtension = function(path, api)
		local extension = script_load_extension(tostring(path));

		if extension ~= nil then
			require(api);
		end

		return extension;
	end,

	UnloadExtension = function(extension)
		script_unload_extension(extension);
	end,

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
		},

		SQLite3 =
		{
			OpenFailed = SCRIPT_EXIT_CODE_SQLITE3_OPEN_FAILED
		}
	}
};
