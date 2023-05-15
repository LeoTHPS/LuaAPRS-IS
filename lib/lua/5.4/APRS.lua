APRS =
{
	IS =
	{
		-- @return aprs_is
		Init = function()
			return aprs_is_init();
		end,

		Deinit = function(aprs_is)
			aprs_is_deinit(aprs_is);
		end,

		IsBlocking = function(aprs_is)
			return aprs_is_is_blocking(aprs_is);
		end,

		IsConnected = function(aprs_is)
			return aprs_is_is_connected(aprs_is);
		end,

		-- @return false on error
		Connect = function(aprs_is, host, port, callsign, passcode, filter)
			return aprs_is_connect(aprs_is, tostring(host), tonumber(port), tostring(callsign), tonumber(passcode), tostring(filter));
		end,

		Disconnect = function(aprs_is)
			aprs_is_disconnect(aprs_is);
		end,

		-- @return would_block, packet
		-- @return packet == nil on connection closed
		ReadPacket = function(aprs_is)
			return aprs_is_read_packet(aprs_is);
		end,

		-- @return false on connection closed
		WritePacket = function(aprs_is, packet)
			return aprs_is_write_packet(aprs_is, packet);
		end,

		-- @return false on connection closed
		SendPacket = function(aprs_is, sender, tocall, digipath, content)
			return aprs_is_send_packet(aprs_is, sender, tocall, digipath, content);
		end,

		SetBlocking = function(aprs_is, value)
			return aprs_is_set_blocking(aprs_is, value);
		end,

		GeneratePasscode = function(callsign)
			return aprs_is_generate_passcode(callsign);
		end
	},

	Packet =
	{
		Init = function(sender, tocall, digipath, content)
			return aprs_packet_init(sender, tocall, digipath, content);
		end,

		Deinit = function(packet)
			aprs_packet_deinit(packet);
		end,

		GetIGate = function(packet)
			return aprs_packet_get_igate(packet);
		end,
		SetIGate = function(packet, value)
			return aprs_packet_set_igate(packet, value);
		end,

		GetQFlag = function(packet)
			return aprs_packet_get_qflag(packet);
		end,
		SetQFlag = function(packet, value)
			return aprs_packet_set_qflag(packet, value);
		end,

		GetToCall = function(packet)
			return aprs_packet_get_tocall(packet);
		end,
		SetToCall = function(packet, value)
			return aprs_packet_set_tocall(packet, value);
		end,

		GetSender = function(packet)
			return aprs_packet_get_sender(packet);
		end,
		SetSender = function(packet, value)
			return aprs_packet_set_sender(packet, value);
		end,

		GetContent = function(packet)
			return aprs_packet_get_content(packet);
		end,
		SetContent = function(packet, value)
			return aprs_packet_set_content(packet, value);
		end,

		GetDigiPath = function(packet)
			return aprs_packet_get_digipath(packet);
		end,
		SetDigiPath = function(packet, value)
			return aprs_packet_set_digipath(packet, value);
		end
	}
};
