Discord =
{
	RichPresence =
	{
		Replies =
		{
			No     = discord_rich_presence_reply_no,
			Yes    = discord_rich_presence_reply_yes,
			Ignore = discord_rich_presence_reply_ignore
		},

		Init = function(application_id, ready, disconnect, errored, join_game, join_request, spectate_game, steam_id)
			if steam_id then
				steam_id = tostring(steam_id);
			end

			return discord_rich_presence_init(tostring(application_id), ready, disconnect, errored, join_game, join_request, spectate_game, steam_id);
		end,

		Deinit = function()
			discord_rich_presence_deinit();
		end,

		Respond = function(user_id, reply)
			discord_rich_presence_respond(tostring(user_id), tonumber(reply));
		end,

		RunCallbacks = function()
			discord_rich_presence_run_callbacks();
		end,

		ClearPresence = function()
			discord_rich_presence_clear_presence();
		end,

		UpdateHandlers = function(ready, disconnect, errored, join_game, join_request, spectate_game)
			discord_rich_presence_update_handlers(ready, disconnect, errored, join_game, join_request, spectate_game);
		end,

		UpdatePresence = function(state, details, timestamp_start, timestamp_stop, large_image_key, large_image_text, small_image_key, small_image_text, party_id, party_size, party_max, match_secret, join_secret, spectate_secret, instance)
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

			discord_rich_presence_update_presence(tostring(state), tostring(details), tonumber(timestamp_start), tonumber(timestamp_stop), large_image_key, large_image_text, small_image_key, small_image_text, tostring(party_id), tonumber(party_size), tonumber(party_max), tostring(match_secret), tostring(join_secret), tostring(spectate_secret), tonumber(instance));
		end,

		UpdateConnection = function()
			if discord_rich_presence_update_connection ~= nil then
				discord_rich_presence_update_connection();
			end
		end
	}
};
