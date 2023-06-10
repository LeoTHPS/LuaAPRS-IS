#define LUA_APRS_IS_EXTENSION

#include "discord_rpc.h"
#include "Extension.hpp"

#include <AL/OS/Thread.hpp>
#include <AL/OS/Window.hpp>
#include <AL/OS/Console.hpp>
#include <AL/OS/Process.hpp>

#include <AL/Game/Loop.hpp>

#include <AL/FileSystem/Path.hpp>

#if defined(AL_PLATFORM_LINUX)
	#define DISCORD_RPC_LIBRARY_NAME "discord-rpc.so"
#elif defined(AL_PLATFORM_WINDOWS)
	#define DISCORD_RPC_LIBRARY_NAME "discord-rpc.dll"
#else
	#error Platform not supported
#endif

const char* process_get_working_directory()
{
	static AL::FileSystem::Path working_directory;

	if (working_directory.GetString().GetLength() != 0)
		return working_directory.GetString().GetCString();
	else
	{
#if defined(AL_PLATFORM_LINUX)
		// TODO: implement
#elif defined(AL_PLATFORM_WINDOWS)
		char buffer[MAX_PATH] = { 0 };

		if (GetCurrentDirectoryA(sizeof(buffer), buffer) != 0)
		{
			working_directory = buffer;

			return working_directory.GetString().GetCString();
		}
#endif
	}

	return nullptr;
}

enum discord_rich_presence_replies : AL::uint8
{
	discord_rich_presence_reply_no     = 0,
	discord_rich_presence_reply_yes    = 1,
	discord_rich_presence_reply_ignore = 2
};

typedef AL::Lua54::Function::LuaCallback<void(const char* user_id, const char* username, const char* discriminator, const char* avatar)> discord_rich_presence_event_handler_ready;
typedef AL::Lua54::Function::LuaCallback<void(int error_code, const char* message)>                                                      discord_rich_presence_event_handler_disconnected;
typedef AL::Lua54::Function::LuaCallback<void(int error_code, const char* message)>                                                      discord_rich_presence_event_handler_errored;
typedef AL::Lua54::Function::LuaCallback<void(const char* join_secret)>                                                                  discord_rich_presence_event_handler_join_game;
typedef AL::Lua54::Function::LuaCallback<void(const char* user_id, const char* username, const char* discriminator, const char* avatar)> discord_rich_presence_event_handler_join_request;
typedef AL::Lua54::Function::LuaCallback<void(const char* spectate_secret)>                                                              discord_rich_presence_event_handler_spectate_game;

struct Discord
{
	decltype(Discord_Initialize)*       Initialize;
	decltype(Discord_Shutdown)*         Shutdown;

	decltype(Discord_RunCallbacks)*     RunCallbacks;

	decltype(Discord_Respond)*          Respond;

	decltype(Discord_ClearPresence)*    ClearPresence;

	decltype(Discord_UpdatePresence)*   UpdatePresence;
	decltype(Discord_UpdateHandlers)*   UpdateHandlers;
#if defined(DISCORD_DISABLE_IO_THREAD)
	decltype(Discord_UpdateConnection)* UpdateConnection;
#endif
};

struct DiscordRichPresenceStorage
{
	AL::String state;
	AL::String details;
	AL::String large_image_key;
	AL::String large_image_text;
	AL::String small_image_key;
	AL::String small_image_text;
	AL::String party_id;
	AL::String match_secret;
	AL::String join_secret;
	AL::String spectate_secret;
};

struct discord_rich_presence
{
	bool                                              is_initialized     = false;
	bool                                              is_thread_stopping = false;

	AL::OS::Process                                   process;
	AL::OS::ProcessLibrary                            library;

	AL::OS::Thread                                    thread;
	AL::OS::Window*                                   window;
	Discord                                           discord;
	DiscordRichPresence                               presence;
	DiscordRichPresenceStorage                        presence_storage;

	DiscordEventHandlers                              event_handlers;

	discord_rich_presence_event_handler_ready         on_ready;
	discord_rich_presence_event_handler_disconnected  on_disconnected;
	discord_rich_presence_event_handler_errored       on_errored;
	discord_rich_presence_event_handler_join_game     on_join_game;
	discord_rich_presence_event_handler_join_request  on_join_request;
	discord_rich_presence_event_handler_spectate_game on_spectate_game;
};

discord_rich_presence discord_rich_presence_instance;

void discord_rich_presence_deinit();

void discord_rich_presence_window_thread_main()
{
	discord_rich_presence_instance.window = new AL::OS::Window(
		"discord_rich_presence_window",
		"discord_rich_presence"
	);

	try
	{
		try
		{
			discord_rich_presence_instance.window->Open();
		}
		catch (AL::Exception& exception)
		{

			throw AL::Exception(
				AL::Move(exception),
				"Error opening AL::OS::Window"
			);
		}
	}
	catch (const AL::Exception& exception)
	{
		delete discord_rich_presence_instance.window;

		AL::OS::Console::WriteException(
			exception
		);

		return;
	}

#if defined(AL_PLATFORM_LINUX)
	// TODO: minimize
#elif defined(AL_PLATFORM_WINDOWS)
	ShowWindow(discord_rich_presence_instance.window->GetHandle(), SW_MINIMIZE);
#endif

	AL::Game::Loop::Run(4, [](AL::TimeSpan _delta)
	{
		if (discord_rich_presence_instance.window->IsOpen())
		{
			try
			{
				try
				{
					discord_rich_presence_instance.window->Update(
						_delta
					);
				}
				catch (AL::Exception& exception)
				{

					throw AL::Exception(
						AL::Move(exception),
						"Error updating AL::OS::Window"
					);
				}
			}
			catch (const AL::Exception& exception)
			{
				AL::OS::Console::WriteException(
					exception
				);

				try
				{
					discord_rich_presence_instance.window->Close();
				}
				catch (const AL::Exception& exception)
				{
					AL::OS::Console::WriteLine(
						"Error closing AL::OS::Window"
					);

					AL::OS::Console::WriteException(
						exception
					);
				}
			}
		}
		else if (discord_rich_presence_instance.is_thread_stopping)
		{
			discord_rich_presence_instance.is_thread_stopping = false;

			return false;
		}

		return true;
	});

	delete discord_rich_presence_instance.window;
}
bool discord_rich_presence_window_thread_start()
{
	try
	{
		try
		{
			discord_rich_presence_instance.thread.Start(
				&discord_rich_presence_window_thread_main
			);
		}
		catch (AL::Exception& exception)
		{

			throw AL::Exception(
				AL::Move(exception),
				"Error starting AL::OS::Thread"
			);
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
void discord_rich_presence_window_thread_stop()
{
	discord_rich_presence_instance.is_thread_stopping = true;

	try
	{
		discord_rich_presence_instance.thread.Join();
	}
	catch (const AL::Exception& exception)
	{
		AL::OS::Console::WriteLine(
			"Error joining AL::OS::Thread"
		);

		AL::OS::Console::WriteException(
			exception
		);
	}

	discord_rich_presence_instance.is_thread_stopping = false;
}

void discord_rich_presence_set_event_handlers(discord_rich_presence_event_handler_ready&& ready, discord_rich_presence_event_handler_disconnected&& disconnect, discord_rich_presence_event_handler_errored&& errored, discord_rich_presence_event_handler_join_game&& join_game, discord_rich_presence_event_handler_join_request&& join_request, discord_rich_presence_event_handler_spectate_game&& spectate_game)
{
	discord_rich_presence_instance.on_ready         = AL::Move(ready);
	discord_rich_presence_instance.on_disconnected  = AL::Move(disconnect);
	discord_rich_presence_instance.on_errored       = AL::Move(errored);
	discord_rich_presence_instance.on_join_game     = AL::Move(join_game);
	discord_rich_presence_instance.on_join_request  = AL::Move(join_request);
	discord_rich_presence_instance.on_spectate_game = AL::Move(spectate_game);
}

bool discord_rich_presence_init(const char* application_id, discord_rich_presence_event_handler_ready ready, discord_rich_presence_event_handler_disconnected disconnect, discord_rich_presence_event_handler_errored errored, discord_rich_presence_event_handler_join_game join_game, discord_rich_presence_event_handler_join_request join_request, discord_rich_presence_event_handler_spectate_game spectate_game, const char* steam_id)
{
	if (discord_rich_presence_instance.is_initialized)
	{

		discord_rich_presence_deinit();
	}

	if (!discord_rich_presence_window_thread_start())
	{

		return false;
	}

	try
	{
		AL::OS::Process::OpenCurrent(
			discord_rich_presence_instance.process
		);

		try
		{
			auto path = AL::FileSystem::Path::Combine(
				process_get_working_directory(),
				DISCORD_RPC_LIBRARY_NAME
			);

			if (!AL::OS::ProcessLibrary::Load(discord_rich_presence_instance.library, discord_rich_presence_instance.process, path))
			{

				throw AL::Exception(
					"File not found"
				);
			}
		}
		catch (AL::Exception& exception)
		{
			discord_rich_presence_instance.process.Close();

			throw AL::Exception(
				AL::Move(exception),
				"Error loading Discord RPC library [Path: " DISCORD_RPC_LIBRARY_NAME " ]"
			);
		}

		try
		{
			#define discord_rich_presence_try_resolve_export(__function__, __name__) \
				if (!discord_rich_presence_instance.library.GetExport(__function__, __name__)) \
				{ \
					\
					throw AL::Exception( \
						"Error resolving " __name__ \
					); \
				}

			discord_rich_presence_try_resolve_export(discord_rich_presence_instance.discord.Initialize,       "Discord_Initialize");
			discord_rich_presence_try_resolve_export(discord_rich_presence_instance.discord.Shutdown,         "Discord_Shutdown");
			discord_rich_presence_try_resolve_export(discord_rich_presence_instance.discord.RunCallbacks,     "Discord_RunCallbacks");
			discord_rich_presence_try_resolve_export(discord_rich_presence_instance.discord.Respond,          "Discord_Respond");
			discord_rich_presence_try_resolve_export(discord_rich_presence_instance.discord.ClearPresence,    "Discord_ClearPresence");
			discord_rich_presence_try_resolve_export(discord_rich_presence_instance.discord.UpdatePresence,   "Discord_UpdatePresence");
			discord_rich_presence_try_resolve_export(discord_rich_presence_instance.discord.UpdateHandlers,   "Discord_UpdateHandlers");
#if defined(DISCORD_DISABLE_IO_THREAD)
			discord_rich_presence_try_resolve_export(discord_rich_presence_instance.discord.UpdateConnection, "Discord_UpdateConnection");
#endif

			#undef discord_rich_presence_try_resolve_export
		}
		catch (AL::Exception& exception)
		{
			discord_rich_presence_instance.library.Unload();
			discord_rich_presence_instance.process.Close();

			throw AL::Exception(
				AL::Move(exception),
				"Error resolving Discord RPC library exports"
			);
		}
	}
	catch (const AL::Exception& exception)
	{
		discord_rich_presence_window_thread_stop();

		AL::OS::Console::WriteException(
			exception
		);

		return false;
	}

	discord_rich_presence_set_event_handlers(AL::Move(ready), AL::Move(disconnect), AL::Move(errored), AL::Move(join_game), AL::Move(join_request), AL::Move(spectate_game));

	discord_rich_presence_instance.event_handlers.ready = [](const DiscordUser* request)
	{
		discord_rich_presence_instance.on_ready(request->userId, request->username, request->discriminator, request->avatar);

		try
		{
			discord_rich_presence_instance.window->Close();
		}
		catch (const AL::Exception& exception)
		{
			AL::OS::Console::WriteLine(
				"Error closing AL::OS::Window"
			);

			AL::OS::Console::WriteException(
				exception
			);
		}
	};
	discord_rich_presence_instance.event_handlers.disconnected = [](int errorCode, const char* message)
	{
		discord_rich_presence_instance.on_disconnected(errorCode, message);
	};
	discord_rich_presence_instance.event_handlers.errored = [](int errorCode, const char* message)
	{
		discord_rich_presence_instance.on_errored(errorCode, message);
	};
	discord_rich_presence_instance.event_handlers.joinGame = [](const char* joinSecret)
	{
		discord_rich_presence_instance.on_join_game(joinSecret);
	};
	discord_rich_presence_instance.event_handlers.spectateGame = [](const char* spectateSecret)
	{
		discord_rich_presence_instance.on_spectate_game(spectateSecret);
	};
	discord_rich_presence_instance.event_handlers.joinRequest = [](const DiscordUser* request)
	{
		discord_rich_presence_instance.on_join_request(request->userId, request->userId, request->discriminator, request->avatar);
	};

	discord_rich_presence_instance.discord.Initialize(application_id, &discord_rich_presence_instance.event_handlers, 0, steam_id);

	discord_rich_presence_instance.is_initialized = true;

	return true;
}
void discord_rich_presence_deinit()
{
	if (discord_rich_presence_instance.is_initialized)
	{
		discord_rich_presence_window_thread_stop();

		discord_rich_presence_instance.on_disconnected(0, "Shutdown");

		discord_rich_presence_instance.discord.Shutdown();
		discord_rich_presence_instance.library.Unload();
		discord_rich_presence_instance.process.Close();

		discord_rich_presence_instance.on_ready.Release();
		discord_rich_presence_instance.on_disconnected.Release();
		discord_rich_presence_instance.on_errored.Release();
		discord_rich_presence_instance.on_join_game.Release();
		discord_rich_presence_instance.on_join_request.Release();
		discord_rich_presence_instance.on_spectate_game.Release();

		discord_rich_presence_instance.is_initialized = false;
	}
}

void discord_rich_presence_respond(const char* user_id, discord_rich_presence_replies reply)
{
	if (discord_rich_presence_instance.is_initialized)
	{
		discord_rich_presence_instance.discord.Respond(user_id, static_cast<int>(reply));
	}
}
void discord_rich_presence_run_callbacks()
{
	if (discord_rich_presence_instance.is_initialized)
	{
		discord_rich_presence_instance.discord.RunCallbacks();
	}
}
void discord_rich_presence_clear_presence()
{
	if (discord_rich_presence_instance.is_initialized)
	{
		discord_rich_presence_instance.discord.ClearPresence();
	}
}
#if defined(DISCORD_DISABLE_IO_THREAD)
void discord_rich_presence_update_connection()
{
	if (discord_rich_presence_instance.is_initialized)
	{
		discord_rich_presence_instance.discord.UpdateConnection();
	}
}
#endif
void discord_rich_presence_update_handlers(discord_rich_presence_event_handler_ready ready, discord_rich_presence_event_handler_disconnected disconnect, discord_rich_presence_event_handler_errored errored, discord_rich_presence_event_handler_join_game join_game, discord_rich_presence_event_handler_join_request join_request, discord_rich_presence_event_handler_spectate_game spectate_game)
{
	if (discord_rich_presence_instance.is_initialized)
	{
		discord_rich_presence_set_event_handlers(AL::Move(ready), AL::Move(disconnect), AL::Move(errored), AL::Move(join_game), AL::Move(join_request), AL::Move(spectate_game));
	}
}
void discord_rich_presence_update_presence(const char* state, const char* details, AL::uint32 timestamp_start, AL::uint32 timestamp_end, const char* large_image_key, const char* large_image_text, const char* small_image_key, const char* small_image_text, const char* party_id, int party_size, int party_max, const char* match_secret, const char* join_secret, const char* spectate_secret, AL::int8 instance)
{
	if (discord_rich_presence_instance.is_initialized)
	{
		discord_rich_presence_instance.presence_storage.state            = state;
		discord_rich_presence_instance.presence_storage.details          = details;
		discord_rich_presence_instance.presence_storage.large_image_key  = large_image_key ? large_image_key : "";
		discord_rich_presence_instance.presence_storage.large_image_text = large_image_text ? large_image_text : "";
		discord_rich_presence_instance.presence_storage.small_image_key  = small_image_key ? small_image_key : "";
		discord_rich_presence_instance.presence_storage.small_image_text = small_image_text ? small_image_text : "";
		discord_rich_presence_instance.presence_storage.party_id         = party_id;
		discord_rich_presence_instance.presence_storage.match_secret     = match_secret;
		discord_rich_presence_instance.presence_storage.join_secret      = join_secret;
		discord_rich_presence_instance.presence_storage.spectate_secret  = spectate_secret;

		discord_rich_presence_instance.presence.state          = discord_rich_presence_instance.presence_storage.state.GetCString();
		discord_rich_presence_instance.presence.details        = discord_rich_presence_instance.presence_storage.details.GetCString();
		discord_rich_presence_instance.presence.startTimestamp = timestamp_start;
		discord_rich_presence_instance.presence.endTimestamp   = timestamp_end;
		discord_rich_presence_instance.presence.largeImageKey  = discord_rich_presence_instance.presence_storage.large_image_key.GetCString();
		discord_rich_presence_instance.presence.largeImageText = discord_rich_presence_instance.presence_storage.large_image_text.GetCString();
		discord_rich_presence_instance.presence.smallImageKey  = discord_rich_presence_instance.presence_storage.small_image_key.GetCString();
		discord_rich_presence_instance.presence.smallImageText = discord_rich_presence_instance.presence_storage.small_image_text.GetCString();
		discord_rich_presence_instance.presence.partyId        = discord_rich_presence_instance.presence_storage.party_id.GetCString();
		discord_rich_presence_instance.presence.partySize      = party_size;
		discord_rich_presence_instance.presence.partyMax       = party_max;
		discord_rich_presence_instance.presence.matchSecret    = discord_rich_presence_instance.presence_storage.match_secret.GetCString();
		discord_rich_presence_instance.presence.joinSecret     = discord_rich_presence_instance.presence_storage.join_secret.GetCString();
		discord_rich_presence_instance.presence.spectateSecret = discord_rich_presence_instance.presence_storage.spectate_secret.GetCString();
		discord_rich_presence_instance.presence.instance       = instance;

		discord_rich_presence_instance.discord.UpdatePresence(&discord_rich_presence_instance.presence);
	}
}

LUA_APRS_IS_EXTENSION_INIT([](Extension& extension)
{
	LUA_APRS_IS_RegisterGlobal(discord_rich_presence_reply_no);
	LUA_APRS_IS_RegisterGlobal(discord_rich_presence_reply_yes);
	LUA_APRS_IS_RegisterGlobal(discord_rich_presence_reply_ignore);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rich_presence_init);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rich_presence_deinit);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rich_presence_respond);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rich_presence_run_callbacks);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rich_presence_clear_presence);
#if defined(DISCORD_DISABLE_IO_THREAD)
	LUA_APRS_IS_RegisterGlobalFunction(discord_rich_presence_update_connection);
#endif
	LUA_APRS_IS_RegisterGlobalFunction(discord_rich_presence_update_handlers);
	LUA_APRS_IS_RegisterGlobalFunction(discord_rich_presence_update_presence);
});

LUA_APRS_IS_EXTENSION_DEINIT([](Extension& extension)
{
	LUA_APRS_IS_UnregisterGlobal(discord_rich_presence_reply_no);
	LUA_APRS_IS_UnregisterGlobal(discord_rich_presence_reply_yes);
	LUA_APRS_IS_UnregisterGlobal(discord_rich_presence_reply_ignore);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rich_presence_init);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rich_presence_deinit);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rich_presence_respond);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rich_presence_run_callbacks);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rich_presence_clear_presence);
#if defined(DISCORD_DISABLE_IO_THREAD)
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rich_presence_update_connection);
#endif
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rich_presence_update_handlers);
	LUA_APRS_IS_UnregisterGlobalFunction(discord_rich_presence_update_presence);
});
