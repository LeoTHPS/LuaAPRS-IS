#pragma once
#include <AL/Common.hpp>

#include <AL/Lua54/Lua.hpp>

#define APRS_API_RegisterGlobal(__variable__)                              APRS_API_RegisterGlobalEx(__variable__, #__variable__)
#define APRS_API_RegisterGlobalEx(__variable__, __variable_name__)         lua.SetGlobal(__variable_name__, __variable__)

#define APRS_API_RegisterGlobalFunction(__function__)                      APRS_API_RegisterGlobalFunctionEx(__function__, #__function__)
#define APRS_API_RegisterGlobalFunctionEx(__function__, __function_name__) lua.SetGlobalFunction<__function__>(__function_name__)

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
			RegisterExtensions();
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

		static void RegisterExtensions();
	};
}
