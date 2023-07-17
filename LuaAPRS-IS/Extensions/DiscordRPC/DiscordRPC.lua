DiscordRPC =
{
	-- @param on_ready      function(user_id, user_name, user_flags, user_premium)
	-- @param on_error      function(error_code, error_message)
	-- @param on_connect    function()
	-- @param on_disconnect function(error_code, error_message)
	Init = function(application_id, on_ready, on_error, on_connect, on_disconnect)
		return discord_rpc_init(tostring(application_id), on_ready, on_error, on_connect, on_disconnect);
	end,

	Deinit = function(discord_rpc)
		discord_rpc_deinit(discord_rpc);
	end,

	-- @return false on connection closed
	Poll = function(discord_rpc)
		return discord_rpc_poll(discord_rpc);
	end,

	-- @return false on connection closed
	UpdatePresence = function(discord_rpc, presence)
		return discord_rpc_update_presence(discord_rpc, presence);
	end,

	Presence =
	{
		Init = function()
			return discord_rpc_presence_init();
		end,

		Deinit = function(presence)
			discord_rpc_presence_deinit(presence);
		end,

		GetHeader = function(presence)
			return discord_rpc_presence_get_header(presence);
		end,

		SetHeader = function(presence, value)
			discord_rpc_presence_set_header(presence, tostring(value));
		end,

		GetDetails = function(presence)
			return discord_rpc_presence_get_details(presence);
		end,

		SetDetails = function(presence, value)
			discord_rpc_presence_set_details(presence, tostring(value));
		end,

		GetTimeStart = function(presence)
			return discord_rpc_presence_get_time_start(presence);
		end,

		SetTimeStart = function(presence, value)
			discord_rpc_presence_set_time_start(presence, tonumber(value));
		end,

		GetTimeEnd = function(presence)
			return discord_rpc_presence_get_time_end(presence);
		end,

		SetTimeEnd = function(presence, value)
			discord_rpc_presence_set_time_end(presence, tonumber(value));
		end,

		GetButton = function(presence, index)
			return discord_rpc_presence_get_button(presence, tonumber(index));
		end,

		GetButtonCount = function(presence)
			return discord_rpc_presence_get_button_count(presence);
		end,

		GetImageLargeKey = function(presence)
			return discord_rpc_presence_get_image_large_key(presence);
		end,

		SetImageLargeKey = function(presence, value)
			discord_rpc_presence_set_image_large_key(presence, tostring(value));
		end,

		GetImageLargeText = function(presence)
			return discord_rpc_presence_get_image_large_text(presence);
		end,

		SetImageLargeText = function(presence, value)
			discord_rpc_presence_set_image_large_text(presence, tostring(value));
		end,

		GetImageSmallKey = function(presence)
			return discord_rpc_presence_get_image_small_key(presence);
		end,

		SetImageSmallKey = function(presence, value)
			discord_rpc_presence_set_image_small_key(presence, tostring(value));
		end,

		GetImageSmallText = function(presence)
			return discord_rpc_presence_get_image_small_text(presence);
		end,

		SetImageSmallText = function(presence, value)
			discord_rpc_presence_set_image_small_text(presence, tostring(value));
		end,

		Button =
		{
			GetURL = function(button)
				return discord_rpc_presence_button_get_url(button);
			end,

			SetURL = function(button, value)
				discord_rpc_presence_button_set_url(button, tostring(value));
			end,

			GetLabel = function(button)
				return discord_rpc_presence_button_get_label(button);
			end,

			SetLabel = function(button, value)
				discord_rpc_presence_button_set_label(button, tostring(value));
			end
		},

		Buttons =
		{
			Add = function(presence, label, url)
				return discord_rpc_presence_buttons_add(presence, tostring(label), tostring(url));
			end,

			Remove = function(presence, button)
				discord_rpc_presence_buttons_remove(presence, button);
			end,

			Clear = function(presence)
				discord_rpc_presence_buttons_clear(presence);
			end
		}
	}
};
