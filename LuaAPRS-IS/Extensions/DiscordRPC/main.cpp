#define LUA_APRS_IS_EXTENSION

#include "Extension.hpp"

#include <DiscordRPC.hpp>

#include <AL/OS/Console.hpp>

typedef DiscordRPC::IPCConnection                                                                                                                          discord_rpc;
typedef DiscordRPC::RichPresence                                                                                                                           discord_rpc_presence;
typedef DiscordRPC::Button                                                                                                                                 discord_rpc_presence_button;

typedef DiscordRPC::IPCConnectionOnReadyEventHandler                                                                                                       discord_rpc_on_ready_event_handler;
typedef DiscordRPC::IPCConnectionOnErrorEventHandler                                                                                                       discord_rpc_on_error_event_handler;
typedef DiscordRPC::IPCConnectionOnConnectEventHandler                                                                                                     discord_rpc_on_connect_event_handler;
typedef DiscordRPC::IPCConnectionOnDisconnectEventHandler                                                                                                  discord_rpc_on_disconnect_event_handler;

typedef AL::Lua54::LuaCallback<void(const char* user_id, const char* user_name, const char* user_username, AL::uint16 user_flags, AL::uint8 user_premium)> lua_discord_rpc_on_ready_event_handler;
typedef AL::Lua54::LuaCallback<void(AL::uint16 error_code, const char* error_message)>                                                                     lua_discord_rpc_on_error_event_handler;
typedef AL::Lua54::LuaCallback<void()>                                                                                                                     lua_discord_rpc_on_connect_event_handler;
typedef AL::Lua54::LuaCallback<void(AL::uint16 error_code, const char* error_message)>                                                                     lua_discord_rpc_on_disconnect_event_handler;

discord_rpc*                 discord_rpc_init(const char* application_id, lua_discord_rpc_on_ready_event_handler on_ready, lua_discord_rpc_on_error_event_handler on_error, lua_discord_rpc_on_connect_event_handler on_connect, lua_discord_rpc_on_disconnect_event_handler on_disconnect)
{
	auto discord_rpc = new ::discord_rpc(application_id);

	discord_rpc->OnReady.Register([discord_rpc, on_ready = AL::Move(on_ready)](const DiscordRPC::User& user)
	{
		return on_ready(user.ID.GetCString(), user.Name.GetCString(), user.Username.GetCString(), static_cast<AL::uint16>(user.Flags.Value), AL::uint8(user.Premium));
	});
	discord_rpc->OnError.Register([discord_rpc, on_error = AL::Move(on_error)](DiscordRPC::ErrorCodes errorCode, const AL::String& errorMessage)
	{
		return on_error(static_cast<AL::uint16>(errorCode), errorMessage.GetCString());
	});
	discord_rpc->OnConnect.Register([discord_rpc, on_connect = AL::Move(on_connect)]()
	{
		return on_connect();
	});
	discord_rpc->OnDisconnect.Register([discord_rpc, on_disconnect = AL::Move(on_disconnect)](DiscordRPC::CloseErrorCodes errorCode, const AL::String& errorMessage)
	{
		return on_disconnect(static_cast<AL::uint16>(errorCode), errorMessage.GetCString());
	});

	try
	{
		if (!discord_rpc->Open())
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
void                         discord_rpc_deinit(discord_rpc* discord_rpc)
{
	if (discord_rpc != nullptr)
	{
		discord_rpc->Close();

		delete discord_rpc;
	}
}

bool                         discord_rpc_poll(discord_rpc* discord_rpc)
{
	if (!discord_rpc || !discord_rpc->IsOpen())
		return false;

	try
	{
		if (!discord_rpc->Poll())
		{
			discord_rpc->Close();

			return false;
		}
	}
	catch (const AL::Exception& exception)
	{
		discord_rpc->Close();

		AL::OS::Console::WriteException(
			exception
		);

		return false;
	}

	return true;
}
bool                         discord_rpc_update_presence(discord_rpc* discord_rpc, discord_rpc_presence* presence)
{
	if (discord_rpc == nullptr)
		return false;

	try
	{
		if (!discord_rpc->UpdatePresence(*presence))
		{

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

discord_rpc_presence*        discord_rpc_presence_init()
{
	auto presence = new discord_rpc_presence
	{
	};

	return presence;
}
void                         discord_rpc_presence_deinit(discord_rpc_presence* presence)
{
	if (presence != nullptr)
		delete presence;
}
const char*                  discord_rpc_presence_get_header(discord_rpc_presence* presence)
{
	return presence ? presence->Header.GetCString() : nullptr;
}
void                         discord_rpc_presence_set_header(discord_rpc_presence* presence, const char* value)
{
	if (presence != nullptr)
		presence->Header = value;
}
const char*                  discord_rpc_presence_get_details(discord_rpc_presence* presence)
{
	return presence ? presence->Details.GetCString() : nullptr;
}
void                         discord_rpc_presence_set_details(discord_rpc_presence* presence, const char* value)
{
	if (presence != nullptr)
		presence->Details = value;
}
AL::uint32                   discord_rpc_presence_get_time_start(discord_rpc_presence* presence)
{
	return static_cast<AL::uint32>(presence ? presence->TimeStart.ToSeconds() : 0);
}
void                         discord_rpc_presence_set_time_start(discord_rpc_presence* presence, AL::uint32 value)
{
	if (presence != nullptr)
		presence->TimeStart = AL::Timestamp::FromSeconds(value);
}
AL::uint32                   discord_rpc_presence_get_time_end(discord_rpc_presence* presence)
{
	return static_cast<AL::uint32>(presence ? presence->TimeEnd.ToSeconds() : 0);
}
void                         discord_rpc_presence_set_time_end(discord_rpc_presence* presence, AL::uint32 value)
{
	if (presence != nullptr)
		presence->TimeEnd = AL::Timestamp::FromSeconds(value);
}
discord_rpc_presence_button* discord_rpc_presence_get_button(discord_rpc_presence* presence, AL::uint32 index)
{
	if (presence != nullptr)
	{
		for (auto& button : presence->Buttons)
		{
			if (--index == 0)
			{

				return &button;
			}
		}
	}

	return nullptr;
}
AL::uint32                   discord_rpc_presence_get_button_count(discord_rpc_presence* presence)
{
	return static_cast<AL::uint32>(presence ? presence->Buttons.GetSize() : 0);
}
const char*                  discord_rpc_presence_get_image_large_key(discord_rpc_presence* presence)
{
	return presence ? presence->ImageLarge.Key.GetCString() : nullptr;
}
void                         discord_rpc_presence_set_image_large_key(discord_rpc_presence* presence, const char* value)
{
	if (presence != nullptr)
		presence->ImageLarge.Key = value;
}
const char*                  discord_rpc_presence_get_image_large_text(discord_rpc_presence* presence)
{
	return presence ? presence->ImageLarge.Text.GetCString() : nullptr;
}
void                         discord_rpc_presence_set_image_large_text(discord_rpc_presence* presence, const char* value)
{
	if (presence != nullptr)
		presence->ImageLarge.Text = value;
}
const char*                  discord_rpc_presence_get_image_small_key(discord_rpc_presence* presence)
{
	return presence ? presence->ImageSmall.Key.GetCString() : nullptr;
}
void                         discord_rpc_presence_set_image_small_key(discord_rpc_presence* presence, const char* value)
{
	if (presence != nullptr)
		presence->ImageSmall.Key = value;
}
const char*                  discord_rpc_presence_get_image_small_text(discord_rpc_presence* presence)
{
	return presence ? presence->ImageSmall.Text.GetCString() : nullptr;
}
void                         discord_rpc_presence_set_image_small_text(discord_rpc_presence* presence, const char* value)
{
	if (presence != nullptr)
		presence->ImageSmall.Text = value;
}
discord_rpc_presence_button* discord_rpc_presence_buttons_add(discord_rpc_presence* presence, const char* label, const char* url)
{
	if (presence != nullptr)
	{
		presence->Buttons.PushBack(
			{
				.URL   = url,
				.Label = label
			}
		);

		return &(*--presence->Buttons.end());
	}

	return nullptr;
}
void                         discord_rpc_presence_buttons_remove(discord_rpc_presence* presence, discord_rpc_presence_button* button)
{
	if (presence != nullptr)
	{
		for (auto it = presence->Buttons.begin(); it != presence->Buttons.end(); ++it)
		{
			if (&(*it) == button)
			{
				presence->Buttons.Erase(
					it
				);

				break;
			}
		}
	}
}
void                         discord_rpc_presence_buttons_clear(discord_rpc_presence* presence)
{
	if (presence != nullptr)
		presence->Buttons.Clear();
}

const char*                  discord_rpc_presence_button_get_url(discord_rpc_presence_button* button)
{
	return button ? button->URL.GetCString() : nullptr;
}
void                         discord_rpc_presence_button_set_url(discord_rpc_presence_button* button, const char* value)
{
	if (button != nullptr)
		button->URL = value;
}
const char*                  discord_rpc_presence_button_get_label(discord_rpc_presence_button* button)
{
	return button ? button->Label.GetCString() : nullptr;
}
void                         discord_rpc_presence_button_set_label(discord_rpc_presence_button* button, const char* value)
{
	if (button != nullptr)
		button->Label = value;
}

LUA_APRS_IS_EXTENSION_INIT([](Extension& extension)
{
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_init);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_deinit);

	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_poll);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_update_presence);

	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_init);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_deinit);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_get_header);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_set_header);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_get_details);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_set_details);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_get_time_start);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_set_time_start);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_get_time_end);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_set_time_end);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_get_button);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_get_button_count);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_get_image_large_key);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_set_image_large_key);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_get_image_large_text);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_set_image_large_text);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_get_image_small_key);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_set_image_small_key);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_get_image_small_text);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_set_image_small_text);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_buttons_add);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_buttons_remove);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_buttons_clear);

	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_button_get_url);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_button_set_url);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_button_get_label);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rpc_presence_button_set_label);
});

LUA_APRS_IS_EXTENSION_DEINIT([](Extension& extension)
{
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_init);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_deinit);

	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_poll);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_update_presence);

	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_init);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_deinit);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_get_header);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_set_header);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_get_details);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_set_details);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_get_time_start);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_set_time_start);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_get_time_end);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_set_time_end);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_get_button);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_get_button_count);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_get_image_large_key);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_set_image_large_key);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_get_image_large_text);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_set_image_large_text);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_get_image_small_key);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_set_image_small_key);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_get_image_small_text);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_set_image_small_text);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_buttons_add);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_buttons_remove);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_buttons_clear);

	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_button_get_url);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_button_set_url);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_button_get_label);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rpc_presence_button_set_label);
});
