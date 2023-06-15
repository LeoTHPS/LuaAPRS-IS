require('Plugins.Gateway');

SkipMonitor = {};

function SkipMonitor.Init(aprs_callsign, aprs_is_passcode, aprs_is_host, aprs_is_port, database_path, threshold_miles)
	if not Gateway.Init(aprs_callsign, aprs_is_passcode, 'WIDE1-1', aprs_is_host, aprs_is_port, 't/poimqstu', database_path) then
		Console.WriteLine('SkipMonitor', 'Error initializing Gateway');
		return false;
	end

	Gateway.Events.RegisterEvent(Gateway.Events.OnReceivePacket, function(packet)
		local packet_sender          = APRS.Packet.GetSender(packet);
		local packet_igate           = APRS.Packet.GetIGate(packet);
		local packet_path            = APRS.Packet.GetDigiPath(packet);
		local packet_path_first_digi = SkipMonitor.Private.GetFirstDigi(packet_path);

		if packet_path_first_digi then
			local packet_path_first_digi_distance_ft = packet_path_first_digi and SkipMonitor.Private.GetStationDistanceToStation(packet_sender, packet_path_first_digi) or nil;

			if packet_path_first_digi_distance_ft then
				local packet_path_first_digi_distance_miles = packet_path_first_digi_distance_ft / 5280;
	
				if packet_path_first_digi_distance_miles >= threshold_miles then
					SkipMonitor.Events.ExecuteEvent(SkipMonitor.Events.OnSkipDetected, packet_sender, packet_path_first_digi, packet_path_first_digi_distance_miles, packet_path, packet_igate);
				end
			end
		else
			local packet_igate_distance_ft = SkipMonitor.Private.GetStationDistanceToStation(packet_sender, packet_igate);

			if packet_igate_distance_ft then
				local packet_igate_distance_miles = packet_igate_distance_ft / 5280;

				if packet_igate_distance_miles >= threshold_miles then
					SkipMonitor.Events.ExecuteEvent(SkipMonitor.Events.OnSkipDetected, packet_sender, packet_igate, packet_igate_distance_miles, packet_path, packet_igate);
				end
			end
		end
	end);

	Gateway.Events.RegisterEvent(Gateway.Events.OnReceivePosition, function(station, path, igate, latitude, longitude, altitude, comment)
		SkipMonitor.Private.SetStationPosition(station, latitude, longitude, altitude);
	end);

	return true;
end

function SkipMonitor.Run(interval_ms)
	if not Gateway.Run(interval_ms) then
		Console.WriteLine('SkipMonitor', 'Error running Gateway');
		return false;
	end

	return true;
end

SkipMonitor.Events                = {};
SkipMonitor.Events.OnSkipDetected = {}; -- function(sender, receiver, distance, path, igate)

function SkipMonitor.Events.ExecuteEvent(event, ...)
	Gateway.Events.ExecuteEvent(event, ...);
end

function SkipMonitor.Events.ScheduleEvent(event, delay, ...)
	Gateway.Events.ScheduleEvent(event, delay, ...);
end

function SkipMonitor.Events.RegisterEvent(event, callback)
	Gateway.Events.RegisterEvent(event, callback);
end

function SkipMonitor.Events.UnregisterEvent(event, callback)
	Gateway.Events.UnregisterEvent(event, callback);
end

SkipMonitor.Private = {};

function SkipMonitor.Private.GetFirstDigi(path)
	return string.match(path, '([^,]+)%*');
end

-- @return exists, latitude, longitude, altitude
function SkipMonitor.Private.GetStatationPosition(station)
	if not SkipMonitor.Private.Stations then
		return false, 0, 0, 0;
	end

	station = SkipMonitor.Private.Stations[station];

	if not station then
		return false, 0, 0, 0;
	end

	return true, station.Latitude, station.Longitude, station.Altitude;
end

function SkipMonitor.Private.SetStationPosition(station, latitude, longitude, altitude)
	if not SkipMonitor.Private.Stations then
		SkipMonitor.Private.Stations = {};
	end

	SkipMonitor.Private.Stations[station] =
	{
		Altitude  = altitude,
		Latitude  = latitude,
		Longitude = longitude
	};
end

function SkipMonitor.Private.GetStationDistanceToStation(station1, station2)
	local station1_exists, station1_latitude, station1_longitude, station1_altitude = SkipMonitor.Private.GetStatationPosition(station1);
	local station2_exists, station2_latitude, station2_longitude, station2_altitude = SkipMonitor.Private.GetStatationPosition(station2);

	if not station1_exists or not station2_exists then
		return nil;
	end

	return SkipMonitor.Private.GetDistanceBetweenPoints(station1_latitude, station1_longitude, station1_altitude, station2_latitude, station2_longitude, station2_altitude);
end

function SkipMonitor.Private.GetDistanceBetweenPoints(latitude1, longitude1, altitude1, latitude2, longitude2, altitude2)
	local latitude_delta  = math.rad(latitude2 - latitude1);
	local longitude_delta = math.rad(longitude2 - longitude1);
	local latitude_1      = math.rad(latitude1);
	local latitude_2      = math.rad(latitude2);
	local a               = math.sin(latitude_delta / 2) * math.sin(latitude_delta / 2) + math.sin(longitude_delta / 2) * math.sin(longitude_delta / 2) * math.cos(latitude_1) * math.cos(latitude_2);
	local distance        = 2 * math.atan(math.sqrt(a), math.sqrt(1 - a));
	local distance_z      = 0;

	-- if altitude1 >= altitude2 then
	-- 	distance_z = altitude1 - altitude2;
	-- else
	-- 	distance_z = altitude2 - altitude1;
	-- end

	return ((distance * 6371) * 3280.84) + distance_z;
end
