require('Plugins.Gateway');

Script.LoadExtension('Extensions.Socket');
Script.LoadExtension('Extensions.ByteBuffer');

-- TODO: alert stations when fire is added (probably should add position expiration)

FireWatch = {};

function FireWatch.Init(aprs_callsign, aprs_is_passcode, aprs_path, aprs_is_host, aprs_is_port, database_path)
	if not Gateway.Init(aprs_callsign, aprs_is_passcode, aprs_path, aprs_is_host, aprs_is_port, 't/p', database_path) then
		Console.WriteLine('FireWatch', 'Error initializing Gateway');
		return false;
	end

	Gateway.Events.RegisterEvent(Gateway.Events.OnConnected, function(host, port, callsign)
		FireWatch.Events.ExecuteEvent(FireWatch.Private.Events.PollSources);
	end);

	Gateway.Events.RegisterEvent(Gateway.Events.OnDisconnected, function()
		FireWatch.Private.Sources.Disconnect();
	end);

	Gateway.Events.RegisterEvent(Gateway.Events.OnReceivePosition, function(station, path, igate, latitude, longitude, altitude, comment)
		local station_exists, station_latitude, station_longitude, station_altitude = FireWatch.Private.GetStationPosition(station);
		local fire_exists, fire_id, fire_name, distance_to_nearest_hotspot, distance_to_nearest_hotspot_elevation = FireWatch.Private.FindFire(latitude, longitude, altitude);

		if fire_exists and station_exists then
			FireWatch.Private.SetStationPosition(station, latitude, longitude, altitude);
			FireWatch.Events.ExecuteEvent(FireWatch.Events.OnStationPositionChangedInFireZone, fire_id, fire_name, station, latitude, longitude, altitude, comment, distance_to_nearest_hotspot, distance_to_nearest_hotspot_elevation);
		elseif fire_exists and not station_exists then
			FireWatch.Private.SetStationPosition(station, latitude, longitude, altitude);
			FireWatch.Events.ExecuteEvent(FireWatch.Events.OnStationEnteredFireZone, fire_id, fire_name, station, latitude, longitude, altitude, comment, distance_to_nearest_hotspot, distance_to_nearest_hotspot_elevation);
		elseif not fire_exists and station_exists then
			local prev_fire_exists, prev_fire_guid, prev_fire_name, prev_distance_to_nearest_hotspot, distance_to_nearest_hotspot_elevation = FireWatch.Private.FindFire(station_latitude, station_longitude, altitude);

			if prev_fire_exists then
				FireWatch.Private.RemoveStationPosition(station);
				FireWatch.Events.ExecuteEvent(FireWatch.Events.OnStationLeftFireZone, prev_fire_guid, prev_fire_name, station, latitude, longitude, altitude, comment);
			end
		end
	end);

	FireWatch.Events.RegisterEvent(FireWatch.Events.OnSourceConnected, function(source_host, source_port)
		Console.WriteLine('FireWatch.Sources', string.format('Connected to source %s:%u', source_host, source_port));
	end);

	FireWatch.Events.RegisterEvent(FireWatch.Events.OnSourceDisconnected, function(source_host, source_port)
		Console.WriteLine('FireWatch.Sources', string.format('Disconnected from source %s:%u', source_host, source_port));
	end);

	FireWatch.Events.RegisterEvent(FireWatch.Events.OnSourceConnectionFailed, function(source_host, source_port)
		-- Console.WriteLine('FireWatch.Sources', string.format('Error connecting to %s:%u', source_host, source_port));
	end);

	FireWatch.Events.RegisterEvent(FireWatch.Private.Events.PollSources, function()
		FireWatch.Events.ScheduleEvent(FireWatch.Private.Events.PollSources, FireWatch.Sources.GetPollInterval());
		FireWatch.Private.Sources.Poll();
	end);

	FireWatch.Events.RegisterEvent(FireWatch.Private.Events.OnReceivePacket, function(opcode, byte_buffer, byte_buffer_size)
		-- @return success, fire_hotspot_latitude, fire_hotspot_longitude, fire_hotspot_altitude, fire_hotspot_radius_ft
		local function ByteBuffer_read_fire_hotspot_position(byte_buffer)
			local fire_hotspot_latitude_exists, fire_hotspot_latitude   = ByteBuffer.ReadFloat(byte_buffer);
			local fire_hotspot_longitude_exists, fire_hotspot_longitude = ByteBuffer.ReadFloat(byte_buffer);
			local fire_hotspot_altitude_exists, fire_hotspot_altitude   = ByteBuffer.ReadInt16(byte_buffer);
			local fire_hotspot_radius_ft_exists, fire_hotspot_radius_ft = ByteBuffer.ReadUInt32(byte_buffer);

			if fire_hotspot_latitude_exists and fire_hotspot_longitude_exists and fire_hotspot_altitude_exists and fire_hotspot_radius_ft_exists then
				return true, fire_hotspot_latitude, fire_hotspot_longitude, fire_hotspot_altitude, fire_hotspot_radius_ft;
			end

			return false, nil, nil, nil, nil;
		end

		-- @return success, fire_hotspot_positions { { .Radius, .Altitude, .Latitude, .Longitude }, ... }
		local function ByteBuffer_read_fire_hotspot_positions(byte_buffer, count)
			local fire_hotspot_positions = {};

			for i = 1, count do
				local success, fire_hotspot_latitude, fire_hotspot_longitude, fire_hotspot_altitude, fire_hotspot_radius_ft = ByteBuffer_read_fire_hotspot_position(byte_buffer);

				if not success then
					return false, nil;
				end

				fire_hotspot_positions[i] =
				{
					Radius    = fire_hotspot_radius_ft,
					Altitude  = fire_hotspot_altitude,
					Latitude  = fire_hotspot_latitude,
					Longitude = fire_hotspot_longitude
				};
			end

			return true, fire_hotspot_positions;
		end

		if opcode == FireWatch.Private.Sources.Protocol.OPCodes.AddFire then
			local fire_id_exists, fire_id                       = ByteBuffer.ReadUInt64(byte_buffer);
			local fire_name_exists, fire_name                   = ByteBuffer.ReadString(byte_buffer);
			local fire_hotspot_count_exists, fire_hotspot_count = ByteBuffer.ReadUInt32(byte_buffer);

			if fire_id_exists and fire_name_exists and fire_hotspot_count_exists then
				local fire_hotspot_positions_success, fire_hotspot_positions = ByteBuffer_read_fire_hotspot_positions(byte_buffer, fire_hotspot_count);

				if fire_hotspot_positions_success then
					FireWatch.Events.ExecuteEvent(FireWatch.Private.Events.OnReceivePacket_AddFire, fire_id, fire_name, fire_hotspot_positions);
				end
			end
		elseif opcode == FireWatch.Private.Sources.Protocol.OPCodes.RemoveFire then
			local fire_id_exists, fire_id = ByteBuffer.ReadUInt64(byte_buffer);

			if fire_id_exists then
				FireWatch.Events.ExecuteEvent(FireWatch.Private.Events.OnReceivePacket_RemoveFire, fire_id);
			end
		elseif opcode == FireWatch.Private.Sources.Protocol.OPCodes.UpdateFire then
			local fire_id_exists, fire_id                       = ByteBuffer.ReadUInt64(byte_buffer);
			local fire_name_exists, fire_name                   = ByteBuffer.ReadString(byte_buffer);
			local fire_hotspot_count_exists, fire_hotspot_count = ByteBuffer.ReadUInt32(byte_buffer);

			if fire_id_exists and fire_name_exists and fire_hotspot_count_exists then
				local fire_hotspot_positions_success, fire_hotspot_positions = ByteBuffer_read_fire_hotspot_positions(byte_buffer, fire_hotspot_count);

				if fire_hotspot_positions_success then
					FireWatch.Events.ExecuteEvent(FireWatch.Private.Events.OnReceivePacket_UpdateFire, fire_id, fire_name, fire_hotspot_positions);
				end
			end
		end
	end);

	FireWatch.Events.RegisterEvent(FireWatch.Private.Events.OnReceivePacket_AddFire, function(fire_id, fire_name, fire_hotspot_positions)
		FireWatch.Private.AddFire(fire_id, fire_name, fire_hotspot_positions);
	end);

	FireWatch.Events.RegisterEvent(FireWatch.Private.Events.OnReceivePacket_RemoveFire, function(fire_id)
		FireWatch.Private.RemoveFire(fire_id);
	end);

	FireWatch.Events.RegisterEvent(FireWatch.Private.Events.OnReceivePacket_UpdateFire, function(fire_id, fire_name, fire_hotspot_positions)
		FireWatch.Private.UpdateFire(fire_id, fire_name, fire_hotspot_positions);
	end);

	return true;
end

function FireWatch.Run(interval_ms)
	if not Gateway.Run(interval_ms) then
		Console.WriteLine('FireWatch', 'Error running Gateway');
		return false;
	end

	return true;
end

-- @return false on connection closed
function FireWatch.SendMessage(station, message)
	return Gateway.APRS.IS.SendMessage(station, message);
end

FireWatch.Private = {};

-- @return exists, id, name, distance_to_nearest_hotspot, distance_to_nearest_hotspot_elevation
function FireWatch.Private.FindFire(latitude, longitude, altitude)
	if FireWatch.Private.Fires then
		for fire_id, fire in pairs(FireWatch.Private.Fires) do
			for fire_hotspot_index, fire_hotspot_position in ipairs(fire.HotSpots) do
				local distance_to_fire_hotspot_position = FireWatch.Private.GetDistanceBetweenPoints(fire_hotspot_position.Latitude, fire_hotspot_position.Longitude, latitude, longitude);

				if distance_to_fire_hotspot_position <= fire_hotspot_position.Radius then
					local distance_to_nearest_hotspot_elevation = (altitude >= fire_hotspot_position.Altitude) and (altitude - fire_hotspot_position.Altitude) or 0;

					return true, fire_id, tostring(fire.Name), distance_to_fire_hotspot_position, distance_to_nearest_hotspot_elevation;
				end
			end
		end
	end

	return false, 0, nil, 0, 0;
end

-- @param id uint64
-- @param name string
-- @param hotspot_positions { { .Radius, .Altitude, .Latitude, .Longitude }, ... }
function FireWatch.Private.AddFire(id, name, hotspot_positions)
	if not FireWatch.Private.Fires then
		FireWatch.Private.Fires = {};
	end

	FireWatch.Private.Fires[id] =
	{
		ID       = id,
		Name     = name,
		HotSpots = hotspot_positions
	};

	FireWatch.Events.ExecuteEvent(FireWatch.Events.OnFireStarted, id, name, hotspot_positions);
end

-- @param id uint64
function FireWatch.Private.RemoveFire(id)
	if FireWatch.Private.Fires then
		local fire = FireWatch.Private.Fires[id];

		if fire then
			FireWatch.Private.Fires[id] = nil;

			FireWatch.Events.ExecuteEvent(FireWatch.Events.OnFireExtinguished, id, fire.Name);
		end
	end
end

-- @param id uint64
-- @param name string
-- @param hotspot_positions { { .Radius, .Altitude, .Latitude, .Longitude }, ... }
function FireWatch.Private.UpdateFire(id, name, hotspot_positions)
	if FireWatch.Private.Fires then
		local fire = FireWatch.Private.Fires[id];

		if fire then
			if fire.Name ~= name then
				local fire_prev_name = fire.Name;

				fire.Name = name;
				FireWatch.Events.ExecuteEvent(FireWatch.Events.OnFireNameChanged, id, name, fire_prev_name);
			end

			fire.HotSpots = hotspot_positions;

			FireWatch.Events.ExecuteEvent(FireWatch.Events.OnFireUpdated, id, name, hotspot_positions);
		end
	end
end

-- @return exists, latitude, longitude, altitude
function FireWatch.Private.GetStationPosition(station)
	if FireWatch.Private.StationPositions then
		station = FireWatch.Private.StationPositions[station];

		if station then
			return true, station.Latitude, station.Longitude, station.Altitude;
		end
	end

	return false, 0, 0, 0;
end

function FireWatch.Private.SetStationPosition(station, latitude, longitude, altitude)
	if not FireWatch.Private.StationPositions then
		FireWatch.Private.StationPositions = {};
	end

	FireWatch.Private.StationPositions[station] =
	{
		Altitude  = tostring(altitude),
		Latitude  = tonumber(latitude),
		Longitude = tonumber(longitude)
	};
end

function FireWatch.Private.RemoveStationPosition(station)
	if FireWatch.Private.StationPositions then
		FireWatch.Private.StationPositions[station] = nil;
	end
end

function FireWatch.Private.GetDistanceBetweenPoints(latitude1, longitude1, latitude2, longitude2)
	local latitude_delta  = math.rad(latitude2 - latitude1);
	local longitude_delta = math.rad(longitude2 - longitude1);
	local latitude_1      = math.rad(latitude1);
	local latitude_2      = math.rad(latitude2);
	local a               = math.sin(latitude_delta / 2) * math.sin(latitude_delta / 2) + math.sin(longitude_delta / 2) * math.sin(longitude_delta / 2) * math.cos(latitude_1) * math.cos(latitude_2);
	local distance        = 2 * math.atan(math.sqrt(a), math.sqrt(1 - a));

	return (distance * 6371) * 3280.84;
end

FireWatch.Sources                                     = {};
FireWatch.Private.Sources                             = {};
FireWatch.Private.Sources.Protocol                    = {};
FireWatch.Private.Sources.Protocol.Endian             = ByteBuffer.Endians.Little;
FireWatch.Private.Sources.Protocol.OPCodes            = {};
FireWatch.Private.Sources.Protocol.OPCodes.AddFire    = 1;
FireWatch.Private.Sources.Protocol.OPCodes.RemoveFire = 2;
FireWatch.Private.Sources.Protocol.OPCodes.UpdateFire = 3;

function FireWatch.Sources.Add(host, port)
	if not FireWatch.Private.Sources.EndPoints then
		FireWatch.Private.Sources.EndPoints = {};
	end

	host = tostring(host);
	port = tonumber(port);

	if host and port then
		if not FireWatch.Private.Sources.EndPoints[host] then
			FireWatch.Private.Sources.EndPoints[host] = {};
		end

		if not FireWatch.Private.Sources.EndPoints[host][port] then
			FireWatch.Private.Sources.EndPoints[host][port] = true;
		end
	end
end

function FireWatch.Sources.Remove(host, port)
	if FireWatch.Private.Sources.EndPoints then
		host = tostring(host);

		for source_host, source_ports in pairs(FireWatch.Private.Sources.EndPoints) do
			if source_host == host then
				if FireWatch.Private.Sources.IsSourceConnected(host, port) then
					FireWatch.Private.Sources.DisconnectSource(host, port);
				end

				FireWatch.Private.Sources.EndPoints[source_host][tonumber(port)] = nil;
				break;
			end
		end
	end
end

function FireWatch.Sources.Clear()
	FireWatch.Sources.Enumerate(function(host, port)
		if FireWatch.Private.Sources.IsSourceConnected(host, port) then
			FireWatch.Private.Sources.DisconnectSource(host, port);
		end
	end);

	if FireWatch.Private.Sources.EndPoints then
		FireWatch.Private.Sources.EndPoints = {};
	end
end

-- @param callback function(host, port)
function FireWatch.Sources.Enumerate(callback)
	if FireWatch.Private.Sources.EndPoints then
		for source_host, source_ports in pairs(FireWatch.Private.Sources.EndPoints) do
			for source_port, _ in pairs(source_ports) do
				callback(source_host, source_port);
			end
		end
	end
end

function FireWatch.Sources.GetPollInterval()
	return FireWatch.Private.Sources.PollInterval or 10;
end

function FireWatch.Sources.SetPollInterval(value)
	FireWatch.Private.Sources.PollInterval = tonumber(value);
end

function FireWatch.Private.Sources.Poll()
	FireWatch.Sources.Enumerate(function(host, port)
		if FireWatch.Private.Sources.IsSourceConnected(host, port) or FireWatch.Private.Sources.ConnectSource(host, port) then
			FireWatch.Private.Sources.PollSource(host, port);
		end
	end);
end

function FireWatch.Private.Sources.IsSourceConnected(host, port)
	if FireWatch.Private.Sources.Sockets then
		local socket = FireWatch.Private.Sources.Sockets[host][port];

		if not socket then
			return false;
		end

		return Socket.IsConnected(socket);
	end

	return false;
end

function FireWatch.Private.Sources.ConnectSource(host, port)
	local socket = Socket.Open(Socket.Types.TCP);

	Socket.SetBlocking(socket, false);

	if not Socket.Connect(socket, host, port) then
		Socket.Close(socket);
		FireWatch.Events.ExecuteEvent(FireWatch.Events.OnSourceConnectionFailed, host, port);
		return false;
	end

	FireWatch.Private.Sources.Sockets[host][port] = socket;

	FireWatch.Events.ExecuteEvent(FireWatch.Events.OnSourceConnected, host, port);
end

function FireWatch.Private.Sources.DisconnectSource(host, port)
	if FireWatch.Private.Sources.Sockets then
		local socket = FireWatch.Private.Sources.Sockets[host][port];

		if socket then
			Socket.Close(socket);
			FireWatch.Private.Sources.Sockets[host][port] = nil;

			FireWatch.Events.ExecuteEvent(FireWatch.Events.OnSourceDisconnected, host, port);
		end
	end
end

function FireWatch.Private.Sources.PollSource(host, port)
	if FireWatch.Private.Sources.Sockets then
		local socket                         = FireWatch.Private.Sources.Sockets[host][port];
		local connection_closed, byte_buffer = Socket.ReceiveAll(socket, FireWatch.Private.Sources.Protocol.Endian, 1 + 4);

		if connection_closed then
			FireWatch.Private.Sources.DisconnectSource(host, port);
		else
			local packet_opcode      = ByteBuffer.ReadUInt8(byte_buffer);
			local packet_buffer_size = ByteBuffer.ReadUInt32(byte_buffer);

			ByteBuffer.Release(byte_buffer);

			connection_closed, byte_buffer = Socket.ReceiveAll(socket, FireWatch.Private.Sources.Protocol.Endian, packet_buffer_size);

			if connection_closed then
				FireWatch.Private.Sources.DisconnectSource(host, port);
			else
				FireWatch.Events.ExecuteEvent(FireWatch.Private.Events.OnReceivePacket, packet_opcode, byte_buffer, packet_buffer_size);
				ByteBuffer.Release(byte_buffer);
			end
		end
	end
end

FireWatch.Events                                    = {};
-- @param fire_hotspot_positions { { .Radius, .Altitude, .Latitude, .Longitude }, ... }
FireWatch.Events.OnFireStarted                      = {}; -- function(fire_id, fire_name, fire_hotspot_positions)
-- @param fire_hotspot_positions { { .Radius, .Altitude, .Latitude, .Longitude }, ... }
FireWatch.Events.OnFireUpdated                      = {}; -- function(fire_id, fire_name, fire_hotspot_positions)
FireWatch.Events.OnFireNameChanged                  = {}; -- function(fire_id, fire_name, fire_prev_name)
FireWatch.Events.OnFireExtinguished                 = {}; -- function(fire_id, fire_name)
FireWatch.Events.OnSourceConnected                  = {}; -- function(source_host, source_port)
FireWatch.Events.OnSourceDisconnected               = {}; -- function(source_host, source_port)
FireWatch.Events.OnSourceConnectionFailed           = {}; -- function(source_host, source_port)
FireWatch.Events.OnStationEnteredFireZone           = {}; -- function(fire_id, fire_name, station, station_latitude, station_longitude, station_altitude, station_comment, station_distance_to_nearest_fire_hotspot, station_distance_to_nearest_fire_hotspot_elevation)
FireWatch.Events.OnStationLeftFireZone              = {}; -- function(fire_id, fire_name, station, station_latitude, station_longitude, station_altitude, station_comment)
FireWatch.Events.OnStationPositionChangedInFireZone = {}; -- function(fire_id, fire_name, station, station_latitude, station_longitude, station_altitude, station_comment, station_distance_to_nearest_fire_hotspot, station_distance_to_nearest_fire_hotspot_elevation)
FireWatch.Private.Events                            = {};
FireWatch.Private.Events.PollSources                = {}; -- function()
FireWatch.Private.Events.OnReceivePacket            = {}; -- function(opcode, byte_buffer, byte_buffer_size)
-- @param fire_hotspot_positions { { .Radius, .Altitude, .Latitude, .Longitude }, ... }
FireWatch.Private.Events.OnReceivePacket_AddFire    = {}; -- function(fire_id, fire_name, fire_hotspot_positions)
FireWatch.Private.Events.OnReceivePacket_RemoveFire = {}; -- function(fire_id)
-- @param fire_hotspot_positions { { .Radius, .Altitude, .Latitude, .Longitude }, ... }
FireWatch.Private.Events.OnReceivePacket_UpdateFire = {}; -- function(fire_id, fire_name, fire_hotspot_positions)

function FireWatch.Events.ExecuteEvent(event, ...)
	Gateway.Events.ExecuteEvent(event, ...);
end

function FireWatch.Events.ScheduleEvent(event, delay, ...)
	Gateway.Events.ScheduleEvent(event, delay, ...);
end

function FireWatch.Events.RegisterEvent(event, callback)
	Gateway.Events.RegisterEvent(event, callback);
end

function FireWatch.Events.UnregisterEvent(event, callback)
	Gateway.Events.UnregisterEvent(event, callback);
end
