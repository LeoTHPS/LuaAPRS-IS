System =
{
	Sleep = function(ms)
		system_sleep(tonumber(ms));
	end,

	GetIdleTime = function()
		return system_get_idle_time();
	end,

	GetTimestamp = function()
		return system_get_timestamp();
	end
};
