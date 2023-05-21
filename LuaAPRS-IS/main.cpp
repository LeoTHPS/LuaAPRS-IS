#include <AL/Common.hpp>

#include <AL/OS/Console.hpp>

#include "API.hpp"

enum EXIT_CODES : int
{
	EXIT_CODE_SUCCESS,
	EXIT_CODE_INVALID_ARGS,
	EXIT_CODE_UNHANDLED_EXCEPTION
};

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		AL::OS::Console::WriteLine(
			"Invalid args"
		);

		AL::OS::Console::WriteLine(
			"%s script.lua",
			argv[0]
		);

		return EXIT_CODE_INVALID_ARGS;
	}

	try
	{
		try
		{
			APRS::API::Init();
		}
		catch (AL::Exception& exception)
		{

			throw AL::Exception(
				AL::Move(exception),
				"Error initializing API"
			);
		}

		try
		{
			if (!APRS::API::LoadScript(argv[1]))
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
	}
	catch (const AL::Exception& exception)
	{
		AL::OS::Console::WriteException(
			exception
		);

		return EXIT_CODE_UNHANDLED_EXCEPTION;
	}

	return EXIT_CODE_SUCCESS;
}
