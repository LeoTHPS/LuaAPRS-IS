#include "API.hpp"
#include "APRS.hpp"

#include <AL/OS/Console.hpp>

#include <AL/Network/DNS.hpp>

#define APRS_API_RegisterGlobal(__lua__, __variable__)                              APRS_API_RegisterGlobalEx(__lua__, __variable__, #__variable__)
#define APRS_API_RegisterGlobalEx(__lua__, __variable__, __variable_name__)         __lua__.SetGlobal(__variable_name__, __variable__)

#define APRS_API_RegisterGlobalFunction(__lua__, __function__)                      APRS_API_RegisterGlobalFunctionEx(__lua__, __function__, #__function__)
#define APRS_API_RegisterGlobalFunctionEx(__lua__, __function__, __function_name__) __lua__.SetGlobalFunction<__function__>(__function_name__)

typedef APRS::Packet aprs_packet;

struct aprs_is
{
	APRS::IS::TcpClient client;
	aprs_packet         packet;
	AL::String          packet_buffer;
};

aprs_is*                                   aprs_is_init()
{
	auto is = new aprs_is
	{
	};

	return is;
}
void                                       aprs_is_deinit(aprs_is* is)
{
	delete is;
}
bool                                       aprs_is_is_blocking(aprs_is* is)
{
	return is->client.IsBlocking();
}
bool                                       aprs_is_is_connected(aprs_is* is)
{
	if (is == nullptr)
	{

		return false;
	}

	return is->client.IsConnected();
}
bool                                       aprs_is_connect(aprs_is* is, const char* host, AL::uint16 port, const char* callsign, AL::uint16 passcode, const char* filter)
{
	if (is == nullptr)
	{

		return false;
	}

	AL::Collections::Array<AL::String> _filter({ filter });
	AL::Network::IPEndPoint            _remoteEP = { .Port = port };
	AL::String                         _callsign(callsign);

	try
	{
		if (!AL::Network::DNS::Resolve(_remoteEP.Host, host))
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

	try
	{
		is->client.Connect(
			_remoteEP,
			_callsign,
			passcode,
			_filter
		);
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
void                                       aprs_is_disconnect(aprs_is* is)
{
	if (is != nullptr)
	{
		is->client.Disconnect();
	}
}
AL::Collections::Tuple<bool, aprs_packet*> aprs_is_read_packet(aprs_is* is)
{
	AL::Collections::Tuple<bool, aprs_packet*> result(false, nullptr);

	if (is != nullptr)
	{
		try
		{
			switch (is->client.ReadPacket(is->packet_buffer, is->packet))
			{
				case -1:
					result.Set<0>(true);
					break;

				default:
					result.Set<1>(&is->packet);
					break;
			}
		}
		catch (const AL::Exception& exception)
		{
			AL::OS::Console::WriteException(
				exception
			);
		}
	}

	return result;
}
bool                                       aprs_is_write_packet(aprs_is* is, aprs_packet* packet)
{
	try
	{
		is->client.WritePacket(
			is->packet_buffer,
			*packet
		);
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
bool                                       aprs_is_set_blocking(aprs_is* is, bool value)
{
	try
	{
		is->client.SetBlocking(
			value
		);
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
AL::uint16                                 aprs_is_generate_passcode(const char* callsign)
{
	return APRS::IS::GeneratePasscode(callsign);
}

const char*                                aprs_packet_get_igate(aprs_packet* packet)
{
	return packet->IGate.GetCString();
}
const char*                                aprs_packet_get_qflag(aprs_packet* packet)
{
	return packet->QFlag.GetCString();
}
const char*                                aprs_packet_get_tocall(aprs_packet* packet)
{
	return packet->ToCall.GetCString();
}
const char*                                aprs_packet_get_sender(aprs_packet* packet)
{
	return packet->Sender.GetCString();
}
const char*                                aprs_packet_get_content(aprs_packet* packet)
{
	return packet->Content.GetCString();
}
const char*                                aprs_packet_get_digipath(aprs_packet* packet)
{
	return packet->DigiPath.GetCString();
}

void APRS::API::RegisterGlobals()
{
	APRS_API_RegisterGlobalFunction(lua, aprs_is_init);
	APRS_API_RegisterGlobalFunction(lua, aprs_is_deinit);
	APRS_API_RegisterGlobalFunction(lua, aprs_is_is_blocking);
	APRS_API_RegisterGlobalFunction(lua, aprs_is_is_connected);
	APRS_API_RegisterGlobalFunction(lua, aprs_is_connect);
	APRS_API_RegisterGlobalFunction(lua, aprs_is_disconnect);
	APRS_API_RegisterGlobalFunction(lua, aprs_is_read_packet);
	// APRS_API_RegisterGlobalFunction(lua, aprs_is_write_packet);
	APRS_API_RegisterGlobalFunction(lua, aprs_is_set_blocking);
	APRS_API_RegisterGlobalFunction(lua, aprs_is_generate_passcode);

	APRS_API_RegisterGlobalFunction(lua, aprs_packet_get_igate);
	APRS_API_RegisterGlobalFunction(lua, aprs_packet_get_qflag);
	APRS_API_RegisterGlobalFunction(lua, aprs_packet_get_tocall);
	APRS_API_RegisterGlobalFunction(lua, aprs_packet_get_sender);
	APRS_API_RegisterGlobalFunction(lua, aprs_packet_get_content);
	APRS_API_RegisterGlobalFunction(lua, aprs_packet_get_digipath);
}
