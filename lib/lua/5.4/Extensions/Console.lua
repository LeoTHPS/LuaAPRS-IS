require('Extensions.System');

Console =
{
	WriteLine = function(module, value)
		if not module or (module == '') then
			print('[' .. System.GetTimestamp() .. '] ' .. value);
		else
			print('[' .. System.GetTimestamp() .. '] [' .. module .. '] ' .. value);
		end
	end
};
