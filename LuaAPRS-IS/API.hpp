#pragma once
#include <AL/Common.hpp>

#include <AL/Lua54/Lua.hpp>

#include "Extensions/Script.hpp"

#define APRS_IS_API_GetGlobal(__variable__)                                   APRS_IS_API_GetGlobalEx(__variable__, #__variable__)
#define APRS_IS_API_GetGlobalEx(__variable__, __variable_name__)              __variable__ = APRS_IS::API::GetGlobal(__variable_name__)

#define APRS_IS_API_SetGlobal(__variable__)                                   APRS_IS_API_SetGlobalEx(__variable__, #__variable__)
#define APRS_IS_API_SetGlobalEx(__variable__, __variable_name__)              APRS_IS::API::SetGlobal(__variable_name__, __variable__)

#define APRS_IS_API_RegisterGlobal(__variable__)                              APRS_IS_API_RegisterGlobalEx(__variable__, #__variable__)
#define APRS_IS_API_RegisterGlobalEx(__variable__, __variable_name__)         lua.SetGlobal(__variable_name__, __variable__)

#define APRS_IS_API_RegisterGlobalFunction(__function__)                      APRS_IS_API_RegisterGlobalFunctionEx(__function__, #__function__)
#define APRS_IS_API_RegisterGlobalFunctionEx(__function__, __function_name__) lua.SetGlobalFunction<__function__>(__function_name__)

namespace APRS_IS
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

		template<typename T>
		static T GetGlobal(const AL::String& name)
		{
			AL_ASSERT(
				IsInitialized(),
				"API not initialized"
			);

			return lua.GetGlobal<T>(name);
		}

		template<typename T>
		static void SetGlobal(const AL::String& name, T value)
		{
			AL_ASSERT(
				IsInitialized(),
				"API not initialized"
			);

			lua.SetGlobal<T>(name, AL::Forward<T>(value));
		}

		// @throw AL::Exception
		// @return false if file does not exist
		static bool LoadScript(const AL::String& file, AL::int16& exitCode)
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

			script_init();

			try
			{
				lua.RunFile(
					path
				);
			}
			catch (AL::Exception& exception)
			{
				script_deinit();

				throw AL::Exception(
					AL::Move(exception),
					"Error running '%s'",
					file.GetCString()
				);
			}

			exitCode = script_get_exit_code();

			script_deinit();

			return true;
		}

	private:
		static void RegisterGlobals();

		static void RegisterExtensions();
	};
}
