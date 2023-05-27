require('Extensions.Mutex');

local date_time_format = '%m/%d/%Y %I:%M:%S %p';

Console =
{
	ReadLine = function(module, prefix, include_date_time)
		Mutex.Lock(Mutex.GetDefaultInstance());

		if include_date_time == nil then
			include_date_time = false;
		end

		if include_date_time then
			io.write('[' .. os.date(date_time_format) .. '] ');
		end

		if module and (module ~= '') then
			io.write('[' .. module .. '] ');
		end

		if prefix and (prefix ~= nil) then
			io.write(prefix .. ': ');
		end

		local value = io.read('l');

		Mutex.Unlock(Mutex.GetDefaultInstance());

		return value;
	end,

	WriteLine = function(module, value)
		Mutex.Lock(Mutex.GetDefaultInstance());

		local date_time = os.date(date_time_format);

		if not module or (module == '') then
			print('[' .. date_time .. '] ' .. value);
		else
			print('[' .. date_time .. '] [' .. module .. '] ' .. value);
		end

		Mutex.Unlock(Mutex.GetDefaultInstance());
	end
};
