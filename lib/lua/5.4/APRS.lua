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

		GeneratePasscode = function(callsign)
			return aprs_is_generate_passcode(callsign);
		end
	},

	Packet =
	{
		GetIGate = function(packet)
			return aprs_packet_get_igate(packet);
		end,

		GetQFlag = function(packet)
			return aprs_packet_get_qflag(packet);
		end,

		GetToCall = function(packet)
			return aprs_packet_get_tocall(packet);
		end,

		GetSender = function(packet)
			return aprs_packet_get_sender(packet);
		end,

		GetContent = function(packet)
			return aprs_packet_get_content(packet);
		end,

		GetDigiPath = function(packet)
			return aprs_packet_get_digipath(packet);
		end
	}
};
