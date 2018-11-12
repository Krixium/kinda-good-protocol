#pragma once

namespace kgp
{
	// Default port
	constexpr short PORT = 42069;

	// Control characters
	namespace PacketType
	{
		constexpr char DATA = 0x02;
		constexpr char ACK = 0x05;
		constexpr char EOT = 0x04;
		constexpr char SYN = 0x15;
	}

	// Packet header
	struct PacketHeader
	{
		char PacketType;
		unsigned int SequenceNumber;
		unsigned int AckNumber;
		unsigned int WindowSize;
	};

	// Sizes
	namespace Size
	{
		constexpr size_t HEADER = sizeof(PacketHeader);
		constexpr size_t PACKET = 1500;
		constexpr size_t DATA = PACKET - HEADER;
		constexpr size_t WINDOW = DATA * 10;
	}

	// Packet
	struct Packet
	{
		struct PacketHeader Header;
		char Data[Size::DATA];
	};

	// Timeouts
	namespace Timeout
	{
		constexpr int IDLE = 10 * 1000;
		constexpr int RCV = 0.5 * 1000;
	}

	// Logging
	constexpr char *LOG_FILE = "kgp.log";

	// Program state
	struct State
	{
		// Waiting for SYN
		bool IDLE;
		// Waiting for ACK for SYN
		bool WAIT_SYN;
		// Waiting for incoming data, EOT, or timeout
		bool WAIT;
		// Waiting for ACK for Data or timeout
		bool DATA_SENT;
		// Has receive timeout been reached
		bool RCV_TO;
		// Has idle timeout been reached
		bool IDLE_TO;
	};
}
