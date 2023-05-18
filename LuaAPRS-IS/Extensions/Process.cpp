#include "Process.hpp"

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
