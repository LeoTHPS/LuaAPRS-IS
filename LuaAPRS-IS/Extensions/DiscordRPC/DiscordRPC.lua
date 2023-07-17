DiscordRPC =
{
	Init = function(application_id)
		return discord_rpc_init(tostring(application_id));
	end,

	Deinit = function(discord_rpc)
		discord_rpc_deinit(discord_rpc);
	end,

	-- @return false on connection closed
	Poll = function(discord_rpc)
		return discord_rpc_poll(discord_rpc);
	end,

	Presence =
	{
		Clear = function(discord_rpc)
			return discord_rpc_presence_clear(discord_rpc);
		end,

		Update = function(discord_rpc, header, details, timestamp_start, timestamp_end, large_image_key, large_image_text, small_image_key, small_image_text)
			if large_image_key then
				large_image_key = tostring(large_image_key);
			end

			if large_image_text then
				large_image_text = tostring(large_image_text);
			end

			if small_image_key then
				small_image_key = tostring(small_image_key);
			end

			if small_image_text then
				small_image_text = tostring(small_image_text);
			end

			return discord_rpc_presence_update(discord_rpc, tostring(header), tostring(details), tonumber(timestamp_start), tonumber(timestamp_end), large_image_key, large_image_text, small_image_key, small_image_text);
		end,

		Refresh = function(discord_rpc)
			return discord_rpc_presence_refresh(discord_rpc);
		end,

		Buttons =
		{
			GetCount = function(discord_rpc)
				return discord_rpc_presence_buttons_get_count(discord_rpc);
			end,

			-- @return success, button_index
			Find = function(discord_rpc, label)
				return discord_rpc_presence_buttons_find(discord_rpc, tostring(label));
			end,

			-- @return success, button_index
			Add = function(discord_rpc, label, url, auto_refresh)
				return discord_rpc_presence_buttons_add(discord_rpc, tostring(label), tostring(url), auto_refresh and true or false);
			end,

			Remove = function(discord_rpc, button_index, auto_refresh)
				return discord_rpc_presence_buttons_remove(discord_rpc, tonumber(button_index), auto_refresh and true or false);
			end,

			Clear = function(discord_rpc, auto_refresh)
				return discord_rpc_presence_buttons_clear(discord_rpc, auto_refresh and true or false);
			end
		}
	}
};
