#define LUA_APRS_IS_EXTENSION

#include "Extension.hpp"

#include <DiscordRPC.hpp>

#include <AL/OS/Console.hpp>

struct discord_rpc
{
	DiscordRPC::RichPresence  rich_presence;
	DiscordRPC::IPCConnection ipc_connection;
};

typedef AL::uint32 discord_rpc_button_index;

discord_rpc*                                           discord_rpc_init(const char* application_id)
{
	auto discord_rpc = new ::discord_rpc
	{
		.ipc_connection = DiscordRPC::IPCConnection(application_id)
	};

	try
	{
		if (!discord_rpc->ipc_connection.Open())
		{

			throw AL::Exception(
				"No connection available"
			);
		}
	}
	catch (const AL::Exception& exception)
	{
		delete discord_rpc;

		AL::OS::Console::WriteException(
			exception
		);

		return nullptr;
	}

	return discord_rpc;
}
void                                                   discord_rpc_deinit(discord_rpc* discord_rpc)
{
	if (discord_rpc != nullptr)
	{
		discord_rpc->ipc_connection.Close();

		delete discord_rpc;
	}
}

bool                                                   discord_rpc_poll(discord_rpc* discord_rpc)
{
	if (!discord_rpc || !discord_rpc->ipc_connection.IsOpen())
		return false;

	try
	{
		if (!discord_rpc->ipc_connection.Poll())
		{
			discord_rpc->ipc_connection.Close();

			return false;
		}
	}
	catch (const AL::Exception& exception)
	{
		discord_rpc->ipc_connection.Close();

		AL::OS::Console::WriteException(
			exception
		);

		return false;
	}

	return true;
}

bool                                                   discord_rpc_presence_refresh(discord_rpc* discord_rpc)
{
	if (!discord_rpc || !discord_rpc->ipc_connection.IsOpen())
		return false;

	try
	{
		if (!discord_rpc->ipc_connection.UpdatePresence(discord_rpc->rich_presence))
		{
			discord_rpc->ipc_connection.Close();

			return false;
		}
	}
	catch (const AL::Exception& exception)
	{
		AL::OS::Console::WriteException(
			exception
		);

		return false;
	}

	return true;
}
bool                                                   discord_rpc_presence_clear(discord_rpc* discord_rpc)
{
	if (discord_rpc == nullptr)
		return false;

	discord_rpc->rich_presence = DiscordRPC::RichPresence();

	return discord_rpc_presence_refresh(discord_rpc);
}
bool                                                   discord_rpc_presence_update(discord_rpc* discord_rpc, const char* header, const char* details, AL::uint32 timestamp_start, AL::uint32 timestamp_end, const char* large_image_key, const char* large_image_text, const char* small_image_key, const char* small_image_text)
{
	if (discord_rpc == nullptr)
		return false;

	static const char STRING_EMPTY[] = "";

	discord_rpc->rich_presence.Header          = header ? header : STRING_EMPTY;
	discord_rpc->rich_presence.Details         = details ? details : STRING_EMPTY;
	discord_rpc->rich_presence.TimeStart       = AL::Timestamp::FromSeconds(timestamp_start);
	discord_rpc->rich_presence.TimeEnd         = AL::Timestamp::FromSeconds(timestamp_end);
	discord_rpc->rich_presence.ImageLarge.Key  = large_image_key ? large_image_key : STRING_EMPTY;
	discord_rpc->rich_presence.ImageLarge.Text = large_image_text ? large_image_text : STRING_EMPTY;
	discord_rpc->rich_presence.ImageSmall.Key  = small_image_key ? small_image_key : STRING_EMPTY;
	discord_rpc->rich_presence.ImageSmall.Text = small_image_text ? small_image_text : STRING_EMPTY;

	return discord_rpc_presence_refresh(discord_rpc);
}

discord_rpc_button_index                               discord_rpc_presence_buttons_get_count(discord_rpc* discord_rpc)
{
	return static_cast<discord_rpc_button_index>(discord_rpc ? discord_rpc->rich_presence.Buttons.GetSize() : 0);
}
// @return success, button_index
AL::Collections::Tuple<bool, discord_rpc_button_index> discord_rpc_presence_buttons_find(discord_rpc* discord_rpc, const char* label)
{
	AL::Collections::Tuple<bool, discord_rpc_button_index> value(false, 0);

	if (discord_rpc != nullptr)
	{
		discord_rpc_button_index button_index = 0;

		for (auto& button : discord_rpc->rich_presence.Buttons)
		{
			if (button.Label.Compare(label, AL::True))
			{
				value.Set<0>(true);
				value.Set<1>(button_index + 1);

				break;
			}

			++button_index;
		}
	}

	return value;
}
// @return success, button_index
AL::Collections::Tuple<bool, discord_rpc_button_index> discord_rpc_presence_buttons_add(discord_rpc* discord_rpc, const char* label, const char* url, bool auto_refresh)
{
	AL::Collections::Tuple<bool, discord_rpc_button_index> value(false, 0);

	if (discord_rpc != nullptr)
	{
		discord_rpc->rich_presence.Buttons.PushBack(
			{
				.URL   = url,
				.Label = label
			}
		);

		value.Set<0>(!auto_refresh || discord_rpc_presence_refresh(discord_rpc));
		value.Set<1>(static_cast<discord_rpc_button_index>(discord_rpc->rich_presence.Buttons.GetSize()));
	}

	return value;
}
bool                                                   discord_rpc_presence_buttons_remove(discord_rpc* discord_rpc, discord_rpc_button_index button_index, bool auto_refresh)
{
	if (discord_rpc == nullptr)
		return false;

	if (discord_rpc->rich_presence.Buttons.GetSize() < button_index)
		return false;

	for (auto it = discord_rpc->rich_presence.Buttons.begin(); it != discord_rpc->rich_presence.Buttons.end(); ++it)
	{
		if (--button_index == 0)
		{
			discord_rpc->rich_presence.Buttons.Erase(
				it
			);

			break;
		}
	}

	return !auto_refresh || discord_rpc_presence_refresh(discord_rpc);
}
bool                                                   discord_rpc_presence_buttons_clear(discord_rpc* discord_rpc, bool auto_refresh)
{
	if (discord_rpc == nullptr)
		return false;

	discord_rpc->rich_presence.Buttons.Clear();

	return !auto_refresh || discord_rpc_presence_refresh(discord_rpc);
}

LUA_APRS_IS_EXTENSION_INIT([](Extension& extension)
{
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_init);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_deinit);

	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_poll);

	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_clear);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_update);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_refresh);

	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_buttons_get_count);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_buttons_find);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_buttons_add);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_buttons_remove);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_buttons_clear);
});

LUA_APRS_IS_EXTENSION_DEINIT([](Extension& extension)
{
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_init);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_deinit);

	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_poll);

	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_clear);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_update);

	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_buttons_get_count);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_buttons_find);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_buttons_add);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_buttons_remove);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_buttons_clear);
});
