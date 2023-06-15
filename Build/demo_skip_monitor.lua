require('Plugins.SkipMonitor');

if SkipMonitor.Init('N0CALL', 0, 'noam.aprs2.net', 14580, 'SkipMonitor.db', 200) then
	SkipMonitor.Events.RegisterEvent(SkipMonitor.Events.OnSkipDetected, function(sender, receiver, distance, path, igate)
		Console.WriteLine(nil, string.format('Received possible skip from %s to %s (~%.2f miles) [Path: %s, IGate: %s]', sender, receiver, distance, path, igate));
	end);

	SkipMonitor.Run();
end
