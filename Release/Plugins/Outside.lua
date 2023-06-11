require('APRS-IS');

require('Extensions.Script');

Script.LoadExtension('Extensions/Mutex.dll',   'Extensions.Mutex');
Script.LoadExtension('Extensions/Types.dll',   'Extensions.Types');
Script.LoadExtension('Extensions/System.dll',  'Extensions.System');
Script.LoadExtension('Extensions/Console.dll', 'Extensions.Console');
Script.LoadExtension('Extensions/Discord.dll', 'Extensions.Discord');
Script.LoadExtension('Extensions/SQLite3.dll', 'Extensions.SQLite3');

Outside          = {};
Outside.Private  = {};
Outside.INFINITE = 0x7FFFFFFFFFFFFFFF;

function Outside.Init(aprs_callsign, aprs_is_passcode, aprs_path, aprs_is_host, aprs_is_port, min_idle_time, position_ttl, database_path, station_list)
	Outside.Private.Config                       = {};
	Outside.Private.Config.MinIdleTime           = min_idle_time;
	Outside.Private.Config.PositionTTL           = position_ttl;
	Outside.Private.Config.APRS                  = {};
	Outside.Private.Config.APRS.Path             = aprs_path;
	Outside.Private.Config.APRS.ToCall           = 'APRS';
	Outside.Private.Config.APRS.Callsign         = aprs_callsign;
	Outside.Private.Config.APRS.IS               = {};
	Outside.Private.Config.APRS.IS.Host          = aprs_is_host;
	Outside.Private.Config.APRS.IS.Port          = aprs_is_port;
	Outside.Private.Config.APRS.IS.Filter        = 'b/' .. aprs_callsign;
	Outside.Private.Config.APRS.IS.Passcode      = aprs_is_passcode;
	Outside.Private.Config.Discord               = {};
	Outside.Private.Config.Discord.ApplicationID = '1108502895051149372';
	Outside.Private.Config.Database              = {};
	Outside.Private.Config.Database.Path         = database_path;

	if station_list then
		Outside.Private.Config.StationList = station_list;

		for station_callsign, station_name in pairs(station_list) do
			Outside.Private.Config.APRS.IS.Filter = Outside.Private.Config.APRS.IS.Filter .. '/' .. station_callsign;
		end
	else
		Console.WriteLine('Outside', 'Warning: No station list defined - *ALL STATIONS* will be accepted');

		if string.lower(Console.ReadLine('Outside', 'Are you sure you want to unleash the spam? Y/N')) ~= 'y' then
			Console.WriteLine('Outside', 'Good choice.');
			return;
		end

		Outside.Private.Config.APRS.IS.Filter = 't/p';
	end

	if not Outside.Private.Database.Open(Outside.Private.Config.Database.Path) then
		Console.WriteLine('Outside', 'Error opening database');

		Script.SetExitCode(Script.ExitCodes.SQLite3.OpenFailed);

		return false;
	end

	if not Outside.Private.Storage.Load() then
		Console.WriteLine('Outside', 'Error loading storage');

		Outside.Private.Database.Close();

		-- TODO: this should have its own error code
		Script.SetExitCode(Script.ExitCodes.SQLite3.OpenFailed);

		return false;
	end

	if Outside.Private.RestoreMostRecentStationLocation() then
		local timestamp, name, callsign, latitude, longitude, altitude, path, igate = Outside.GetMostRecentStationPosition();
		local timestamp_delta                                                       = System.GetTimestamp() - timestamp;

		Console.WriteLine('Outside', string.format('Restored position of %s from %s ago', callsign, Outside.Private.FormatTimestampDeltaAsString(timestamp_delta)));
	end

	Outside.Events.RegisterEvent(Outside.Events.OnReceivePacket, function(packet)
		if APRS.Packet.IsPosition(packet) then
			local packet_position = APRS.Position.Decode(packet);

			if packet_position then
				local packet_sender = APRS.Packet.GetSender(packet);

				local function update_station_position(packet_sender_name)
					local packet_path               = APRS.Packet.GetDigiPath(packet);
					local packet_igate              = APRS.Packet.GetIGate(packet);
					local packet_position_altitude  = APRS.Position.GetAltitude(packet_position);
					local packet_position_latitude  = APRS.Position.GetLatitude(packet_position);
					local packet_position_longitude = APRS.Position.GetLongitude(packet_position);

					Outside.Private.SetStationPosition(System.GetTimestamp(), packet_sender_name, packet_sender, packet_position_latitude, packet_position_longitude, packet_position_altitude, packet_path, packet_igate);
				end

				if not Outside.Private.Config.StationList then
					update_station_position(packet_sender);
				else
					for station_callsign, station_name in pairs(Outside.Private.Config.StationList) do
						if station_callsign == packet_sender then
							update_station_position(station_name);
							break;
						end
					end
				end

				APRS.Position.Deinit(packet_position);
			end
		end
	end);

	Outside.Events.RegisterEvent(Outside.Events.OnPositionChanged, function(station, station_name, path, igate, latitude, longitude, altitude)
		Outside.Private.Storage.SetLastPosition(station_name, station, path, igate, latitude, longitude, altitude);

		if not Outside.Private.SaveStoragePending then
			Outside.Private.SaveStoragePending = true;
			Outside.Events.ScheduleEvent(Outside.Private.Events.OnDoSaveStorage, 1);
		end
	end);

	Outside.Events.RegisterEvent(Outside.Private.Events.OnDoSaveStorage, function()
		Console.WriteLine('Outside', 'Saving storage');

		Outside.Private.SaveStoragePending = false;

		if not Outside.Private.Storage.Save() then
			Console.WriteLine('Outside', 'Error saving storage');
		end
	end);

	Outside.Events.RegisterEvent(Outside.Private.Events.OnDoUpdateDiscord, function()
		if Outside.Private.IsIdle() then
			local discord_header                  = nil;
			local discord_message                 = nil;
			local discord_icon_big                = ((tonumber(os.date('%H')) >= 6) and (tonumber(os.date('%H')) < 21)) and 'outside_day' or 'outside_night';
			local discord_icon_big_text           = nil;
			local discord_icon_small              = nil;
			local discord_icon_small_text         = nil;
			local discord_party_size              = 1;
			local discord_party_size_max          = 0;
	
			local latitude, longitude, altitude                                                                                                         = Outside.GetPosition();
			local station_timestamp, station_name, station_callsign, station_latitude, station_longitude, station_altitude, station_path, station_igate = Outside.GetMostRecentStationPosition();
	
			if station_timestamp == 0 then
				discord_header, discord_message = Outside.GetDefaultIdleMessage();
			else
				local station_distance                                    = Outside.Private.GetDistanceBetweenPoints(latitude, longitude, altitude, station_latitude, station_longitude, station_altitude);
				local idle_message_header, idle_message, distance_divider = Outside.Private.GetIdleMessageByPosition(station_latitude, station_longitude);
	
				if not idle_message_header or not idle_message then
					idle_message_header, idle_message, distance_divider = Outside.Private.GetIdleMessageByDistance(station_distance);
				end
	
				if not idle_message_header or not idle_message then
					idle_message_header, idle_message, distance_divider = Outside.GetDefaultIdleMessage();
				end
	
				local path_icon_station, path_icon, path_icon_comment = Outside.Private.GetStationPathIcon(station_path);

				discord_header          = idle_message_header;
				discord_message         = string.format(idle_message, station_distance / distance_divider);
				discord_icon_big_text   = string.format('Position updated %s ago via %s', Outside.Private.FormatTimestampDeltaAsString(System.GetTimestamp() - station_timestamp), station_name);
				discord_icon_small      = path_icon or 'aprs_icon';
				discord_icon_small_text = path_icon_comment and string.format(path_icon_comment, station_igate) or string.format('%s via %s', station_path, station_igate);
			end
	
			Outside.Private.Discord.RichPresence.Update(discord_header, discord_message, Outside.Private.IdleTimestamp, 0, discord_icon_big, discord_icon_big_text, discord_icon_small, discord_icon_small_text, discord_party_size, discord_party_size_max);
			Outside.Private.Discord.RichPresence.Poll();

			Outside.Events.ScheduleEvent(Outside.Private.Events.OnDoUpdateDiscord, 1);
		end
	end);

	return true;
end

function Outside.Run(tick_rate)
	if not tick_rate then
		tick_rate = 5;
	end

	if not Outside.Private.Database.IsOpen() then
		Console.WriteLine('Outside.Database', 'Database not open');

		return false;
	end

	if not Outside.Private.APRS.IS.Connect(Outside.Private.Config.APRS.IS.Host, Outside.Private.Config.APRS.IS.Port, Outside.Private.Config.APRS.Callsign, Outside.Private.Config.APRS.IS.Passcode, Outside.Private.Config.APRS.Path, Outside.Private.Config.APRS.IS.Filter) then
		Console.WriteLine('Outside.APRS.IS', 'Connection failed');

		Outside.Private.Database.Close();

		Script.SetExitCode(Script.ExitCodes.APRS.IS.ConnectionFailed);

		return false;
	end

	Console.WriteLine('Outside.APRS.IS', 'Connected to ' .. Outside.Private.Config.APRS.IS.Host .. ':' .. Outside.Private.Config.APRS.IS.Port .. ' as ' .. Outside.Private.Config.APRS.Callsign);

	Script.Loop(tick_rate, function(delta_ms)
		if not Outside.Private.APRS.IS.IsConnected() then
			Console.WriteLine('Outside.APRS.IS', 'Connection closed');
			return false;
		end

		repeat
			local aprs_is_would_block, aprs_packet = Outside.Private.APRS.IS.ReceivePacket();

			if not aprs_is_would_block and aprs_packet then
				Outside.Events.ExecuteEvent(Outside.Events.OnReceivePacket, aprs_packet);
			end
		until aprs_is_would_block or not aprs_packet;

		Outside.Private.UpdateIdleState();
		Outside.Events.ExecuteScheduledEvents();

		return true;
	end);

	Outside.Private.LeaveIdleState();

	Outside.Private.APRS.IS.Disconnect();

	if not Outside.Private.Storage.Save() then
		Console.WriteLine('Outside', 'Error saving storage');
	end

	Outside.Private.Database.Close();

	return true;
end

-- @return latitude, longitude, altitude
function Outside.GetPosition()
	if not Outside.Private.Position then
		return 0, 0, 0;
	end

	return Outside.Private.Position.Latitude, Outside.Private.Position.Longitude, Outside.Private.Position.Altitude;
end

function Outside.SetPosition(latitude, longitude, altitude)
	Outside.Private.Position =
	{
		Altitude  = altitude,
		Latitude  = latitude,
		Longitude = longitude
	};
end

-- @return timestamp, name, callsign, latitude, longitude, altitude, path, igate
function Outside.GetMostRecentStationPosition()
	if not Outside.Private.StationPositions then
		return 0, nil, nil, 0, 0, 0, nil, nil;
	end

	local most_recent_station_callsign  = nil;
	local most_recent_station_timestamp = 0;

	for station_callsign, station in pairs(Outside.Private.StationPositions) do
		if (System.GetTimestamp() - station.Timestamp) <= Outside.Private.Config.PositionTTL then
			if station.Timestamp > most_recent_station_timestamp then
				most_recent_station_callsign  = station_callsign;
				most_recent_station_timestamp = station.Timestamp;
			end
		end
	end

	local station = Outside.Private.StationPositions[most_recent_station_callsign];

	if not station then
		return 0, nil, nil, 0, 0, 0, nil, nil;
	end

	return station.Timestamp, tostring(station.Name), tostring(most_recent_station_callsign), tonumber(station.Latitude), tonumber(station.Longitude), tonumber(station.Altitude), tostring(station.Path), tostring(station.IGate);
end

function Outside.Private.SetStationPosition(timestamp, name, callsign, latitude, longitude, altitude, path, igate)
	if not Outside.Private.StationPositions then
		Outside.Private.StationPositions = {};
	end

	Outside.Private.StationPositions[callsign] =
	{
		Timestamp = timestamp,
		Name      = name,
		Path      = path,
		IGate     = igate,
		Altitude  = altitude,
		Latitude  = latitude,
		Longitude = longitude
	};

	Outside.Events.ExecuteEvent(Outside.Events.OnPositionChanged, callsign, name, path, igate, latitude, longitude, altitude);
end

-- @return header, message, distance_divider
function Outside.Private.GetIdleMessageByDistance(value)
	if not Outside.Private.IdleMessagesByDistance then
		return nil, nil, 1;
	end

	local nearest_position_index = 0;
	local nearest_position_delta = 0;

	for i, idle_message in ipairs(Outside.Private.IdleMessagesByDistance) do
		if value >= idle_message.MinDistance then
			local idle_message_distance_delta = value - idle_message.MinDistance;

			if (idle_message_distance_delta < nearest_position_delta) or (nearest_position_index == 0) then
				nearest_position_index = i;
				nearest_position_delta = idle_message_distance_delta;
			end
		end
	end

	local idle_message = Outside.Private.IdleMessagesByDistance[nearest_position_index];

	if not idle_message then
		return nil, nil, 1;
	end

	return tostring(idle_message.Header), tostring(idle_message.Message), idle_message.Divider;
end

function Outside.AddIdleMessageByDistance(min_distance, header, message, distance_divider)
	if not Outside.Private.IdleMessagesByDistance then
		Outside.Private.IdleMessagesByDistance = {};
	end

	table.insert(Outside.Private.IdleMessagesByDistance, {
		Header      = header,
		Message     = message,
		Divider     = distance_divider,
		MinDistance = min_distance
	});
end

-- @return header, message, distance_divider
function Outside.Private.GetIdleMessageByPosition(latitude, longitude)
	if not Outside.Private.IdleMessagesByPosition then
		return nil, nil, 1;
	end

	local nearest_position_index                = 0;
	local nearest_position_distance_delta       = 0;
	local nearest_position_distance_is_infinite = false;

	for i, idle_message in ipairs(Outside.Private.IdleMessagesByPosition) do
		local idle_message_distance = Outside.Private.GetDistanceBetweenPoints(latitude, longitude, 0, idle_message.Latitude, idle_message.Longitude, 0);

		if not nearest_position_distance_is_infinite then
			if idle_message_distance == Outside.INFINITE then
				nearest_position_index                = i;
				nearest_position_distance_delta       = idle_message_distance;
				nearest_position_distance_is_infinite = true;
			elseif idle_message_distance <= idle_message.Radius then
				local idle_message_distance_delta = idle_message.Radius - idle_message_distance;
	
				if (idle_message_distance_delta < nearest_position_distance_delta) or (nearest_position_index == 0) then
					nearest_position_index          = i;
					nearest_position_distance_delta = idle_message_distance_delta;
				end
			end
		elseif idle_message_distance < nearest_position_distance_delta then
			nearest_position_index          = i;
			nearest_position_distance_delta = idle_message_distance;
		end
	end

	local idle_message = Outside.Private.IdleMessagesByPosition[nearest_position_index];

	if not idle_message then
		return nil, nil, 1;
	end

	return tostring(idle_message.Header), tostring(idle_message.Message), idle_message.Divider;
end

function Outside.AddIdleMessageByPosition(latitude, longitude, radius, header, message, distance_divider)
	if not Outside.Private.IdleMessagesByPosition then
		Outside.Private.IdleMessagesByPosition = {};
	end

	table.insert(Outside.Private.IdleMessagesByPosition, {
		Header    = header,
		Message   = message,

		Radius    = radius,
		Divider   = distance_divider,
		Latitude  = latitude,
		Longitude = longitude
	});
end

-- @return header, message, distance_divider
function Outside.GetDefaultIdleMessage()
	local idle_message = Outside.Private.DefaultIdleMessage;

	if not idle_message then
		return '', '', 1;
	end

	return idle_message.Header, idle_message.Message, idle_message.Divider;
end

function Outside.SetDefaultIdleMessage(header, message, distance_divider)
	Outside.Private.DefaultIdleMessage =
	{
		Header  = header,
		Message = message,
		Divider = distance_divider
	};
end

-- @return station, icon, comment
function Outside.Private.GetStationPathIcon(path)
	if not Outside.Private.StationPathIcons then
		return nil, nil, nil;
	end

	for station in string.gmatch(path, '[^,]+') do
		local icon = Outside.Private.StationPathIcons[station];

		if icon then
			return station, tostring(icon.Icon), tostring(icon.Comment);
		end
	end

	return nil, nil, nil;
end

function Outside.AddStationPathIcon(station, icon, comment)
	if not Outside.Private.StationPathIcons then
		Outside.Private.StationPathIcons = {};
	end

	Outside.Private.StationPathIcons[station] =
	{
		Icon    = icon,
		Comment = comment
	};
end

function Outside.Private.IsIdle()
	return Outside.Private.IdleTimestamp ~= nil;
end

function Outside.Private.UpdateIdleState()
	if not Outside.Private.IsIdle() then
		if System.GetIdleTime() >= Outside.Private.Config.MinIdleTime then
			Outside.Private.EnterIdleState();
		end
	elseif System.GetIdleTime() < Outside.Private.Config.MinIdleTime then
		Outside.Private.LeaveIdleState();
	end
end

function Outside.Private.EnterIdleState()
	Outside.Private.IdleTimestamp = System.GetTimestamp();

	if not Outside.Private.Discord.RichPresence.Init(Outside.Private.Config.Discord.ApplicationID) then
		Outside.Private.IdleTimestamp = nil;

		return false;
	end

	Outside.Events.ExecuteEvent(Outside.Events.OnEnterIdleState);
	Outside.Events.ExecuteEvent(Outside.Private.Events.OnDoUpdateDiscord);

	return true;
end

function Outside.Private.LeaveIdleState()
	Outside.Events.ExecuteEvent(Outside.Events.OnLeaveIdleState);

	Outside.Private.Discord.RichPresence.Deinit();

	Outside.Private.IdleTimestamp = nil;
end

function Outside.Private.RestoreMostRecentStationLocation()
	local timestamp, name, station, path, igate, latitude, longitude, altitude = Outside.Private.Storage.GetLastPosition();

	if not station or ((System.GetTimestamp() - timestamp) > Outside.Private.Config.PositionTTL) then
		return false;
	end

	Outside.Private.SetStationPosition(timestamp, name, station, latitude, longitude, altitude, path, igate);

	return true;
end

function Outside.Private.FormatTimestampDeltaAsString(delta)
	local function is_plural(value)
		return (value < 1) or (value >= 2);
	end

	if delta < 60 then
		return string.format('%.0f second%s', delta, is_plural(delta) and 's' or '');
	elseif delta < 3600 then
		local delta_minutes = delta / 60;
		local delta_seconds = delta - (math.floor(delta_minutes) * 60);

		return string.format('%.0f minute%s and %.0f second%s', delta_minutes, is_plural(delta_minutes) and 's' or '', delta_seconds, is_plural(delta_seconds) and 's' or '');
	end

	local delta_hours   = delta / 3600;
	local delta_minutes = (delta - (math.floor(delta_hours) * 3600)) / 60;

	return string.format('%.0f hour%s and %.0f minute%s', delta_hours, is_plural(delta_hours) and 's' or '', delta_minutes, is_plural(delta_minutes) and 's' or '');
end

function Outside.Private.GetDistanceBetweenPoints(latitude1, longitude1, altitude1, latitude2, longitude2, altitude2)
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

Outside.Events                           = {};
Outside.Events.OnEnterIdleState          = {}; -- function()
Outside.Events.OnLeaveIdleState          = {}; -- function()
Outside.Events.OnDiscordError            = {}; -- function(error_code, message)
Outside.Events.OnDiscordConnected        = {}; -- function(user_id, username, discriminator)
Outside.Events.OnDiscordDisconnected     = {}; -- function(error_code, message)
Outside.Events.OnReceivePacket           = {}; -- function(packet)
Outside.Events.OnPositionChanged         = {}; -- function(station, station_name, path, igate, latitude, longitude, altitude)
Outside.Private.Events                   = {};
Outside.Private.Events.OnDoSaveStorage   = {}; -- function()
Outside.Private.Events.OnDoUpdateDiscord = {}; -- function()

function Outside.Events.ExecuteEvent(event, ...)
	for event_index, event_callback in ipairs(event) do
		event_callback(...);
	end
end

function Outside.Events.ScheduleEvent(event, delay, ...)
	if not Outside.Private.ScheduledEvents then
		Outside.Private.ScheduledEvents = {};
	end

	local timestamp = System.GetTimestamp() + delay;

	if not Outside.Private.ScheduledEvents[timestamp] then
		Outside.Private.ScheduledEvents[timestamp] = {};
	end

	table.insert(Outside.Private.ScheduledEvents[timestamp], { event, table.pack(...) });
end

function Outside.Events.RegisterEvent(event, callback)
	table.insert(event, callback);
end

function Outside.Events.UnregisterEvent(event, callback)
	for event_index, event_callback in ipairs(event) do
		if event_callback == callback then
			table.remove(event, event_index);

			break;
		end
	end
end

function Outside.Events.ExecuteScheduledEvents()
	while Outside.Private.Events.ExecuteScheduledEvents() do
		-- do nothing
	end
end

function Outside.Private.Events.ExecuteScheduledEvents()
	if Outside.Private.ScheduledEvents then
		for timestamp, events in pairs(Outside.Private.ScheduledEvents) do
			if timestamp <= System.GetTimestamp() then
				for event_index, event in ipairs(events) do
					Outside.Events.ExecuteEvent(event[1], table.unpack(event[2]));
				end

				Outside.Private.ScheduledEvents[timestamp] = nil;
				return true;
			end
		end
	end

	return false
end

Outside.Private.APRS                = {};
Outside.Private.APRS.IS             = {};
Outside.Private.APRS.IS.Handle      = nil;
Outside.Private.APRS.IS.PacketCount = 0;

function Outside.Private.APRS.IS.IsConnected()
	return Outside.Private.APRS.IS.Handle ~= nil;
end

function Outside.Private.APRS.IS.Connect(host, port, callsign, passcode, path, filter)
	if Outside.Private.APRS.IS.IsConnected() then
		return false;
	end

	Outside.Private.APRS.IS.Handle = APRS.IS.Init(callsign, passcode, filter, path);

	if not Outside.Private.APRS.IS.Handle then
		return false;
	end

	if not APRS.IS.Connect(Outside.Private.APRS.IS.Handle, host, port) then
		APRS.IS.Deinit(Outside.Private.APRS.IS.Handle);
		Outside.Private.APRS.IS.Handle = nil;

		return false;
	end

	return true;
end

function Outside.Private.APRS.IS.Disconnect()
	if Outside.Private.APRS.IS.IsConnected() then
		APRS.IS.Disconnect(Outside.Private.APRS.IS.Handle);
		APRS.IS.Deinit(Outside.Private.APRS.IS.Handle);
		Outside.Private.APRS.IS.Handle = nil;
	end
end

-- @return false on connection closed
function Outside.Private.APRS.IS.SendPacket(content)
	if not Outside.Private.APRS.IS.IsConnected() then
		return false;
	end

	if not APRS.IS.SendPacket(Outside.Private.APRS.IS.Handle, Outside.Private.Config.APRS.ToCall, content) then
		Outside.Private.APRS.IS.Disconnect();

		return false;
	end

	return true;
end

-- @return false on connection closed
function Outside.Private.APRS.IS.SendMessage(destination, content, ack)
	if not Outside.Private.APRS.IS.IsConnected() then
		return false;
	end

	if not APRS.IS.SendMessage(Outside.Private.APRS.IS.Handle, Outside.Private.Config.APRS.ToCall, destination, content, ack) then
		Outside.Private.APRS.IS.Disconnect();

		return false;
	end

	return true;
end

-- @return false on connection closed
function Outside.Private.APRS.IS.SendMessageAck(destination, value)
	if not Outside.Private.APRS.IS.IsConnected() then
		return false;
	end

	if not APRS.IS.SendMessageAck(Outside.Private.APRS.IS.Handle, Outside.Private.Config.APRS.ToCall, destination, value) then
		Outside.Private.APRS.IS.Disconnect();

		return false;
	end

	return true;
end

-- @return would_block, packet
-- @return false, nil on connection closed
function Outside.Private.APRS.IS.ReceivePacket()
	local would_block, packet = APRS.IS.ReadPacket(Outside.Private.APRS.IS.Handle);

	if not would_block and not packet then
		Outside.Private.APRS.IS.Disconnect();

		return false, nil;
	end

	return would_block, packet;
end

Outside.Private.Database        = {};
Outside.Private.Database.Handle = nil;

function Outside.Private.Database.IsOpen()
	return Outside.Private.Database.Handle ~= nil;
end

function Outside.Private.Database.GetColumnName(query_result_row, index)
	return SQLite3.QueryResultRow.GetColumnName(query_result_row, index);
end

function Outside.Private.Database.GetColumnValue(query_result_row, index)
	return SQLite3.QueryResultRow.GetColumnValue(query_result_row, index);
end

function Outside.Private.Database.TableExists(name)
	if not Outside.Private.Database.IsOpen() then
		Console.WriteLine('Outside.Database', 'Not open');
		return false;
	end

	local table_exists = false;

	local function callback(query_result_row, query_result_row_index, query_result_row_column_count)
		if query_result_row_column_count == 1 then
			table_exists = true;
		end
	end

	if not Outside.Private.Database.ExecuteQuery(string.format('SELECT name FROM sqlite_master WHERE type=\'table\' AND name=\'%s\'', name), callback) then
		Console.WriteLine('Outside.Database', 'Error reading sqlite_master');
		return false;
	end

	return table_exists;
end

function Outside.Private.Database.Open(path)
	if Outside.Private.Database.IsOpen() then
		Console.WriteLine('Outside.Database', 'Already open');
		return false;
	end

	Outside.Private.Database.Handle = SQLite3.Init(path, SQLite3.Flags.Create | SQLite3.Flags.ReadWrite);

	if not Outside.Private.Database.Handle then
		Console.WriteLine('Outside.Database', 'Error initializing SQLite3');
		return false;
	end

	if not SQLite3.Open(Outside.Private.Database.Handle) then
		SQLite3.Deinit(Outside.Private.Database.Handle);
		Outside.Private.Database.Handle = nil;

		Console.WriteLine('Outside.Database', string.format('Error opening SQLite3 database \'%s\'', path));

		return false;
	end

	return true;
end

function Outside.Private.Database.Close()
	if Outside.Private.Database.IsOpen() then
		SQLite3.Deinit(Outside.Private.Database.Handle);
		Outside.Private.Database.Handle = nil;
	end
end

-- callback function(query_result_row, query_result_row_index, query_result_row_column_count)
function Outside.Private.Database.ExecuteQuery(query, callback)
	if not Outside.Private.Database.IsOpen() then
		Console.WriteLine('Outside.Database', 'Not open');
		return false;
	end

	return SQLite3.ExecuteQuery(Outside.Private.Database.Handle, query, function(sqlite3, query_result_row, query_result_row_index)
		local query_result_row_column_count = SQLite3.QueryResultRow.GetColumnCount(query_result_row);

		callback(query_result_row, query_result_row_index, query_result_row_column_count);
	end);
end

function Outside.Private.Database.ExecuteNonQuery(query)
	if not Outside.Private.Database.IsOpen() then
		Console.WriteLine('Outside.Database', 'Not open');
		return false;
	end

	return SQLite3.ExecuteNonQuery(Outside.Private.Database.Handle, query);
end

Outside.Private.Discord                        = {};
Outside.Private.Discord.RichPresence           = {};
Outside.Private.Discord.RichPresence.Callbacks = {};

function Outside.Private.Discord.RichPresence.Init(application_id)
	return Discord.RichPresence.Init(application_id, Outside.Private.Discord.RichPresence.Callbacks.Ready, Outside.Private.Discord.RichPresence.Callbacks.Disconnect, Outside.Private.Discord.RichPresence.Callbacks.Error, Outside.Private.Discord.RichPresence.Callbacks.JoinGame, Outside.Private.Discord.RichPresence.Callbacks.JoinRequest, Outside.Private.Discord.RichPresence.Callbacks.SpectateGame, nil);
end

function Outside.Private.Discord.RichPresence.Deinit()
	Discord.RichPresence.ClearPresence();
	Discord.RichPresence.UpdateConnection();
	Discord.RichPresence.RunCallbacks();

	Discord.RichPresence.Deinit();
end

function Outside.Private.Discord.RichPresence.Poll()
	Discord.RichPresence.UpdateConnection();
	Discord.RichPresence.RunCallbacks();
end

function Outside.Private.Discord.RichPresence.Update(header, message, timestamp_start, timestamp_stop, large_image_key, large_image_text, small_image_key, small_image_text, party_size, party_size_max)
	Discord.RichPresence.UpdatePresence(tostring(message or ''), tostring(header or ''), tonumber(timestamp_start), tonumber(timestamp_stop or 0), tostring(large_image_key or ''), tostring(large_image_text or ''), tostring(small_image_key or ''), tostring(small_image_text or ''), '00', tonumber(party_size or 1), tonumber(party_size_max or 0), '11', '22', '33', 1);
end

function Outside.Private.Discord.RichPresence.Callbacks.Ready(user_id, username, discriminator, avatar)
	Outside.Events.ExecuteEvent(Outside.Events.OnDiscordConnected, user_id, username, discriminator);
end

function Outside.Private.Discord.RichPresence.Callbacks.Disconnect(error_code, message)
	Outside.Events.ExecuteEvent(Outside.Events.OnDiscordDisconnected, error_code, message);
end

function Outside.Private.Discord.RichPresence.Callbacks.Error(error_code, message)
	Outside.Events.ExecuteEvent(Outside.Events.OnDiscordError, error_code, message);
end

function Outside.Private.Discord.RichPresence.Callbacks.JoinGame(join_secret)
end

function Outside.Private.Discord.RichPresence.Callbacks.JoinRequest(user_id, username, discriminator, avatar)
	Discord.RichPresence.Respond(user_id, Discord.RichPresence.Replies.No);
end

function Outside.Private.Discord.RichPresence.Callbacks.SpectateGame(spectate_secret)
end

Outside.Storage                    = {};
Outside.Private.Storage            = {};
Outside.Private.Storage.TableNames =
{
	Public  = 'storage_public',
	Private = 'storage_private'
};

function Outside.Storage.Get(key)
	if not Outside.Private.Storage.Public then
		return nil;
	end

	return Outside.Private.Storage.Public[key];
end

function Outside.Storage.Set(key, value)
	if not Outside.Private.Storage.Public then
		Outside.Private.Storage.Public = {};
	end

	Outside.Private.Storage.Public[key] = value;
end

function Outside.Private.Storage.Get(key)
	if not Outside.Private.Storage.Private then
		return nil;
	end

	return Outside.Private.Storage.Private[key];
end

function Outside.Private.Storage.Set(key, value)
	if not Outside.Private.Storage.Private then
		Outside.Private.Storage.Private = {};
	end

	Outside.Private.Storage.Private[key] = value;
end

-- @return timestamp, name, station, path, igate, latitude, longitude, altitude
function Outside.Private.Storage.GetLastPosition()
	local last_position = Outside.Private.Storage.Get('last_position');

	if not last_position then
		return 0, nil, nil, nil, nil, 0, 0, 0;
	end

	local last_position_name      = Outside.Private.Storage.Get('last_position_name');
	local last_position_path      = Outside.Private.Storage.Get('last_position_path');
	local last_position_igate     = Outside.Private.Storage.Get('last_position_igate');
	local last_position_station   = Outside.Private.Storage.Get('last_position_station');
	local last_position_altitude  = Outside.Private.Storage.Get('last_position_altitude');
	local last_position_latitude  = Outside.Private.Storage.Get('last_position_latitude');
	local last_position_longitude = Outside.Private.Storage.Get('last_position_longitude');
	local last_position_timestamp = Outside.Private.Storage.Get('last_position_timestamp');

	return tonumber(last_position_timestamp), tostring(last_position_name), tostring(last_position_station), tostring(last_position_path), tostring(last_position_igate), tonumber(last_position_latitude), tonumber(last_position_longitude), tonumber(last_position_altitude);
end

function Outside.Private.Storage.SetLastPosition(name, station, path, igate, latitude, longitude, altitude)
	Outside.Private.Storage.Set('last_position',           true);
	Outside.Private.Storage.Set('last_position_name',      tostring(name));
	Outside.Private.Storage.Set('last_position_path',      tostring(path));
	Outside.Private.Storage.Set('last_position_igate',     tostring(igate));
	Outside.Private.Storage.Set('last_position_station',   tostring(station));
	Outside.Private.Storage.Set('last_position_altitude',  tonumber(altitude));
	Outside.Private.Storage.Set('last_position_latitude',  tonumber(latitude));
	Outside.Private.Storage.Set('last_position_longitude', tonumber(longitude));
	Outside.Private.Storage.Set('last_position_timestamp', System.GetTimestamp());
end

function Outside.Private.Storage.Load()
	local public_success, public_table   = Outside.Private.Storage.LoadTable(Outside.Private.Storage.TableNames.Public);
	local private_success, private_table = Outside.Private.Storage.LoadTable(Outside.Private.Storage.TableNames.Private);

	if not public_success then
		Console.WriteLine('Outside.Storage', 'Error loading public');
		return false;
	end

	if not private_success then
		Console.WriteLine('Outside.Storage', 'Error loading private');
		return false;
	end

	Outside.Private.Storage.Public  = public_table;
	Outside.Private.Storage.Private = private_table;

	return true;
end

function Outside.Private.Storage.Save()
	if not Outside.Private.Storage.SaveTable(Outside.Private.Storage.TableNames.Public, Outside.Private.Storage.Public) then
		Console.WriteLine('Outside.Storage', 'Error saving public');
		return false;
	end

	if not Outside.Private.Storage.SaveTable(Outside.Private.Storage.TableNames.Private, Outside.Private.Storage.Private) then
		Console.WriteLine('Outside.Storage', 'Error saving private');
		return false;
	end

	return true;
end

-- @return success, table
function Outside.Private.Storage.LoadTable(name)
	local table = nil;

	if not Outside.Private.Database.TableExists(name) then
		return true, table;
	else
		local function callback(query_result_row, query_result_row_index, query_result_row_column_count)
			table = {};

			for i = 1, query_result_row_column_count do
				local column_name         = Outside.Private.Database.GetColumnName(query_result_row, i);
				local column_value        = Outside.Private.Database.GetColumnValue(query_result_row, i);
				local column_value_prefix = string.sub(column_value, 1, 1);
				local column_value_data   = string.sub(column_value, 2);

				if column_value_prefix == 's' then
					table[column_name] = tostring(column_value_data);
				elseif (column_value_prefix == 'i') or (column_value_prefix == 'f') then
					table[column_name] = tonumber(column_value_data);
				elseif column_value_prefix == 'b' then
					table[column_name] = (tonumber(column_value_data) ~= 0) and true or false;
				end
			end
		end

		if not Outside.Private.Database.ExecuteQuery(string.format('SELECT * FROM \'%s\'', name), callback) then
			Console.WriteLine('Outside.Storage', string.format('Error reading table %s', name));

			return false, table;
		end
	end

	return true, table;
end

function Outside.Private.Storage.SaveTable(name, value)
	if value then
		local columns       = '';
		local column_count  = 1;
		local column_names  = '';
		local column_values = '';

		for key, value in pairs(value) do
			if type(key) == 'string' then
				if column_count > 1 then
					columns       = columns .. ', ';
					column_names  = column_names .. ', ';
					column_values = column_values .. ', ';
				end

				columns      = columns .. string.format("'%s'", key);
				column_count = column_count + 1;
				column_names = column_names .. string.format("'%s' TEXT NOT NULL", key);

				local column_value_type = type(value);

				if column_value_type == 'string' then
					column_values = column_values .. string.format("'s%s'", value);
				elseif column_value_type == 'number' then
					if (value % 1) == 0 then
						column_values = column_values .. string.format("'i%i'", value);
					else
						column_values = column_values .. string.format("'f%f'", value);
					end
				elseif column_value_type == 'boolean' then
					column_values = column_values .. string.format("'b%u'", value and 1 or 0);
				end
			end
		end

		if not Outside.Private.Database.ExecuteNonQuery(string.format('DROP TABLE IF EXISTS \'%s\'', name)) then
			Console.WriteLine('Outside.Storage', string.format('Error dropping table %s', name));

			return false;
		end

		if not Outside.Private.Database.ExecuteNonQuery(string.format('CREATE TABLE \'%s\' (%s)', name, column_names)) then
			Console.WriteLine('Outside.Storage', string.format('Error creating table %s', name));

			return false;
		end

		if not Outside.Private.Database.ExecuteNonQuery(string.format('INSERT INTO %s (%s) VALUES(%s)', name, columns, column_values)) then
			Console.WriteLine('Outside.Storage', string.format('Error writing table %s', name));

			return false;
		end
	end

	return true;
end
