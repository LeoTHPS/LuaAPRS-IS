#include <AL/Common.hpp>

#include <AL/OS/Console.hpp>

#include "API.hpp"

enum EXIT_CODE_ERRORS : AL::int16
{
	EXIT_CODE_ERROR_BASE_SUCCESS,
	EXIT_CODE_ERROR_BASE_INVALID_ARGS,
	EXIT_CODE_ERROR_BASE_UNHANDLED_EXCEPTION
};

enum EXIT_CODE_ERROR_SOURCES : AL::uint8
{
	EXIT_CODE_ERROR_SOURCE_BASE,
	EXIT_CODE_ERROR_SOURCE_SCRIPT
};

struct ExitCode
{
	AL::uint8 Source;
	AL::int16 ErrorCode;

	operator int() const
	{
		return (static_cast<int>(Source) << 16) | static_cast<int>(ErrorCode);
	}
};

int main(int argc, char* argv[])
{
	ExitCode exitCode =
	{
		.Source    = EXIT_CODE_ERROR_SOURCE_BASE,
		.ErrorCode = EXIT_CODE_ERROR_BASE_SUCCESS
	};

	if (argc != 2)
	{
		AL::OS::Console::WriteLine(
			"Invalid args"
		);

		AL::OS::Console::WriteLine(
			"%s script.lua",
			argv[0]
		);

		exitCode.ErrorCode = EXIT_CODE_ERROR_BASE_INVALID_ARGS;

		return exitCode;
	}

	try
	{
		try
		{
			APRS::IS::API::Init();
		}
		catch (AL::Exception& exception)
		{

			throw AL::Exception(
				AL::Move(exception),
				"Error initializing API"
			);
		}

		AL::int16 scriptExitCode;

		try
		{
			if (!APRS::IS::API::LoadScript(argv[1], scriptExitCode))
			{

				throw AL::Exception(
					"File not found"
				);
			}
		}
		catch (AL::Exception& exception)
		{

			throw AL::Exception(
				AL::Move(exception),
				"Error loading script [Path: %s]",
				argv[1]
			);
		}

		if (scriptExitCode != SCRIPT_EXIT_CODE_SUCCESS)
		{
			exitCode.Source    = EXIT_CODE_ERROR_SOURCE_SCRIPT;
			exitCode.ErrorCode = scriptExitCode;
		}
	}
	catch (const AL::Exception& exception)
	{
		AL::OS::Console::WriteException(
			exception
		);

		exitCode.ErrorCode = EXIT_CODE_ERROR_BASE_UNHANDLED_EXCEPTION;
	}

	return exitCode;
}
