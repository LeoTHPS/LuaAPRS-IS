#pragma once
#include <AL/Common.hpp>

#include <AL/Collections/Array.hpp>

#include <AL/Network/TcpSocket.hpp>
#include <AL/Network/SocketExtensions.hpp>

namespace APRS
{
	struct Packet
	{
		AL::String IGate;
		AL::String QFlag;
		AL::String ToCall;
		AL::String Sender;
		AL::String Content;
		AL::String DigiPath;

		AL::String Encode() const
		{
			return AL::String::Format(
				"%s>%s,%s:%s",
				Sender.GetCString(),
				ToCall.GetCString(),
				DigiPath.GetCString(),
				Content.GetCString()
			);
		}

		static bool Decode(Packet& packet, const AL::String& string)
		{
			if (!string.StartsWith('#'))
			{
				AL::Regex::MatchCollection matches;

				if (AL::Regex::Match(matches, "^([^>]+?)>([^,]+?),((?:([^,]+),)*?)(q[A-Z]{2}),([^:]+):(.+)$", string))
				{
					packet =
					{
						.IGate    = AL::Move(matches[6]),
						.QFlag    = AL::Move(matches[5]),
						.ToCall   = AL::Move(matches[2]),
						.Sender   = AL::Move(matches[1]),
						.Content  = AL::Move(matches[7]),
						.DigiPath = AL::Move(matches[3])
					};

					if (packet.DigiPath.GetLength() != 0)
					{
						packet.DigiPath.Erase(
							--packet.DigiPath.end()
						);
					}

					return true;
				}
			}

			return false;
		}
	};

	namespace IS
	{
		class TcpClient
		{
			bool                    isBlocking  = false;
			bool                    isConnected = false;

			AL::Network::TcpSocket* lpSocket;

		public:
			TcpClient()
			{
			}

			virtual ~TcpClient()
			{
				if (IsConnected())
				{

					Disconnect();
				}
			}

			bool IsBlocking() const
			{
				return isBlocking;
			}

			bool IsConnected() const
			{
				return isConnected;
			}

			// @throw AL::Exception
			void Connect(const AL::Network::IPEndPoint& remoteEP, const AL::String& callsign, AL::uint16 passcode, const AL::Collections::Array<AL::String>& filter)
			{
				AL_ASSERT(
					!IsConnected(),
					"TcpClient already connected"
				);

				lpSocket = new AL::Network::TcpSocket(
					remoteEP.Host.GetFamily()
				);

				// this only throws if the socket is already open
				lpSocket->SetBlocking(IsBlocking());

				try
				{
					lpSocket->Open();
				}
				catch (AL::Exception& exception)
				{
					delete lpSocket;

					throw AL::Exception(
						AL::Move(exception),
						"Error opening AL::Network::TcpSocket"
					);
				}

				try
				{
					if (!lpSocket->Connect(remoteEP))
					{

						throw AL::Exception(
							"Connection timed out"
						);
					}
				}
				catch (AL::Exception& exception)
				{
					lpSocket->Close();

					delete lpSocket;

					throw AL::Exception(
						AL::Move(exception),
						"Error connecting to %s",
						remoteEP.Host.ToString().GetCString(),
						remoteEP.Port
					);
				}

				isConnected = true;

				try
				{
					SendAuthentication(
						callsign,
						passcode,
						filter
					);
				}
				catch (AL::Exception& exception)
				{
					lpSocket->Close();

					delete lpSocket;

					throw AL::Exception(
						AL::Move(exception),
						"Error sending authentication"
					);
				}
			}

			void Disconnect()
			{
				if (IsConnected())
				{
					lpSocket->Close();

					delete lpSocket;

					isConnected = false;
				}
			}

			// @throw AL::Exception
			// @return 0 on error
			// @return -1 if would block
			int ReadPacket(AL::String& line, Packet& packet)
			{
				AL_ASSERT(
					IsConnected(),
					"TcpClient not connected"
				);

				if (!ReadLine(line, false))
				{

					return -1;
				}

				if (!Packet::Decode(packet, line))
				{
					if (line.StartsWith('#'))
					{

						return -1;
					}

					return 0;
				}

				return 1;
			}

			// @throw AL::Exception
			void WritePacket(AL::String& line, const Packet& packet)
			{
				AL_ASSERT(
					IsConnected(),
					"TcpClient not connected"
				);

				line = packet.Encode();

				try
				{
					WriteLine(
						line
					);
				}
				catch (AL::Exception& exception)
				{

					throw AL::Exception(
						AL::Move(exception),
						"Error sending Packet [Buffer: %s]",
						line.GetCString()
					);
				}
			}

			// @throw AL::Exception
			void SetBlocking(bool value)
			{
				isBlocking = value;

				if (IsConnected())
				{
					lpSocket->SetBlocking(
						value
					);
				}
			}

		private:
			// @throw AL::Exception
			void SendAuthentication(const AL::String& callsign, AL::uint16 passcode, const AL::Collections::Array<AL::String>& filter)
			{
				auto line = [&callsign, passcode, &filter]()
				{
					AL::StringBuilder sb;
					sb << "user " << callsign << " pass " << passcode;

					if (filter.GetSize() != 0)
					{
						sb << " filter";

						for (auto& f : filter)
							sb << ' ' << f;
					}

					return sb.ToString();
				}();

				try
				{
					WriteLine(
						line
					);
				}
				catch (AL::Exception& exception)
				{

					throw AL::Exception(
						AL::Move(exception),
						"Error sending authentication"
					);
				}
			}

		private:
			// @throw AL::Exception
			// @return false if would block
			bool ReadLine(AL::String& value, bool block)
			{
				AL_ASSERT(
					IsConnected(),
					"TcpClient not connected"
				);

				value.Clear();

				AL::String::Char buffer[2];
				AL::size_t       numberOfBytesReceived;

				auto buffer_Update = [&buffer]()
				{
					if ((buffer[0] == '\r') && (buffer[1] == '\n'))
					{

						return false;
					}

					buffer[0] = buffer[1];

					return true;
				};

				if (!block)
				{
					try
					{
						if (!AL::Network::SocketExtensions::TryReceiveAll(*lpSocket, &buffer[1], sizeof(buffer[1]), numberOfBytesReceived))
						{

							throw AL::Exception(
								"AL::Network::TcpSocket unexpectedly closed"
							);
						}
					}
					catch (AL::Exception&)
					{
						Disconnect();

						throw;
					}

					if (numberOfBytesReceived == 0)
					{

						return false;
					}

					value.Append(
						buffer[1]
					);

					buffer_Update();
				}

				do
				{
					try
					{
						if (!AL::Network::SocketExtensions::ReceiveAll(*lpSocket, &buffer[1], sizeof(buffer[1]), numberOfBytesReceived))
						{

							throw AL::Exception(
								"AL::Network::TcpSocket unexpectedly closed"
							);
						}
					}
					catch (AL::Exception&)
					{
						Disconnect();

						throw;
					}

					value.Append(
						buffer[1]
					);
				} while (buffer_Update());

				value.Erase(
					value.GetLength() - 2,
					2
				);

				// AL::OS::Console::WriteLine("-> %s", value.GetCString());

				return true;
			}

			// @throw AL::Exception
			void WriteLine(const AL::String& value)
			{
				AL_ASSERT(
					IsConnected(),
					"TcpClient not connected"
				);

				auto valueLength = value.GetLength();

				if (valueLength > 510)
				{

					valueLength = 510;
				}

				AL::size_t numberOfBytesSent;

				static constexpr char EOL[] = "\r\n";

				try
				{
					if (!AL::Network::SocketExtensions::SendAll(*lpSocket, value.GetCString(), valueLength, numberOfBytesSent) ||
						!AL::Network::SocketExtensions::SendAll(*lpSocket, EOL, sizeof(EOL) - 1, numberOfBytesSent))
					{

						throw AL::Exception(
							"AL::Network::TcpSocket unexpectedly closed"
						);
					}
				}
				catch (AL::Exception&)
				{
					Disconnect();

					throw;
				}

				// AL::OS::Console::WriteLine("<- %s", value.GetCString());
			}
		};

		static AL::uint16 GeneratePasscode(const AL::String& callsign)
		{
			AL::uint16 value = 0x73E2;

			auto lpCallsign = callsign.GetCString();

			for (AL::size_t i = 0; i < callsign.GetLength(); i += 2)
			{
				if (*lpCallsign == '-')
				{

					break;
				}

				value ^= *(lpCallsign++) << 8;
				value ^= *(lpCallsign++);
			}

			return value & 0x7FFF;
		}
	}
}
