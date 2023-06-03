#define LUA_APRS_IS_EXTENSION

#include "Extension.hpp"

#include <AL/FileSystem/Path.hpp>

const char* process_get_root_directory()
{
	static AL::FileSystem::Path root_directory;

	if (root_directory.GetString().GetLength() != 0)
		return root_directory.GetString().GetCString();
	else
	{
#if defined(AL_PLATFORM_LINUX)
		// TODO: implement
#elif defined(AL_PLATFORM_WINDOWS)
		if (auto hModule = GetModuleHandleA(NULL))
		{
			char path[AL_MAX_PATH];

			GetModuleFileNameA(hModule, path, AL_MAX_PATH);

			if (AL::OS::GetLastError() == ERROR_SUCCESS)
			{
				root_directory = AL::FileSystem::Path(path).GetRootPath();

				return root_directory.GetString().GetCString();
			}
		}
#endif
	}

	return nullptr;
}

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

LUA_APRS_IS_EXTENSION_INIT([](Extension& extension)
{
	LUA_APRS_IS_RegisterGlobalFunction(process_get_root_directory);
	LUA_APRS_IS_RegisterGlobalFunction(process_get_working_directory);
});

LUA_APRS_IS_EXTENSION_DEINIT([](Extension& extension)
{
	LUA_APRS_IS_UnregisterGlobalFunction(process_get_root_directory);
	LUA_APRS_IS_UnregisterGlobalFunction(process_get_working_directory);
});
