#pragma once
#include <AL/Common.hpp>

#include <AL/Lua54/Lua.hpp>

namespace APRS
{
	class API
	{
		inline static AL::Lua54::State lua;

		API() = delete;

	public:
		static bool IsInitialized()
		{
			return lua.IsCreated();
		}

		// @throw AL::Exception
		static void Init()
		{
			try
			{
				lua.Create();
			}
			catch (AL::Exception& exception)
			{

				throw AL::Exception(
					AL::Move(exception),
					"Error creating AL::Lua54::State"
				);
			}

			try
			{
				if (!lua.LoadLibrary(AL::Lua54::Libraries::All))
				{

					throw AL::Exception(
						"AL::Lua::Libraries::All not found"
					);
				}
			}
			catch (AL::Exception& exception)
			{
				lua.Destroy();

				throw AL::Exception(
					AL::Move(exception),
					"Error loading AL::Lua54::Libraries::All"
				);
			}

			RegisterGlobals();
		}

		// @throw AL::Exception
		// @return false if file does not exist
		static bool LoadScript(const AL::String& file)
		{
			AL_ASSERT(
				IsInitialized(),
				"API not initialized"
			);

			AL::FileSystem::Path path(
				file
			);

			try
			{
				if (!path.Exists())
				{

					return false;
				}
			}
			catch (AL::Exception& exception)
			{

				throw AL::Exception(
					AL::Move(exception),
					"Error checking if file exists"
				);
			}

			try
			{
				lua.RunFile(
					path
				);
			}
			catch (AL::Exception& exception)
			{

				throw AL::Exception(
					AL::Move(exception),
					"Error running '%s'",
					file.GetCString()
				);
			}

			return true;
		}

	private:
		static void RegisterGlobals();
	};
}
