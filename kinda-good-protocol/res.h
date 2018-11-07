#pragma once

namespace kgp
{
	// Default port
	constexpr short PORT = 42069;

	// Control characters
	constexpr char DATA = 0x02;
	constexpr char ACK = 0x05;
	constexpr char EOT = 0x04;
	constexpr char SYN = 0x15;

	// Packet header
	struct PacketHeader
	{
		char PacketType;
		unsigned int SequenceNumber;
		unsigned int AckNumber;
		unsigned int WindowSize;
	};

	// Sizes
	namespace size
	{
		constexpr size_t HEADER = sizeof(PacketHeader);
		constexpr size_t PACKET = 1500;
		constexpr size_t DATA = PACKET - HEADER;
	}

	// Packet
	struct Packet
	{
		struct PacketHeader Header;
		char Data[size::DATA];
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
		// SYN received, sent for ACK
		bool IDLE;
		// SYN sent, waiting for ACk
		bool RDY_SND;
		// SYN established, waiting for ACK or DATA
		bool WAIT;
		// Received data
		bool DATA_RCV;
		// Data sent, waiting for ACK
		bool DATA_SEND;
		// Data sent, no ACK received before timeout
		bool RESEND;
		// Receive timeout reached
		bool RX_TOR;
		// WAIT timeout reached
		bool IDLE_TOR;
	};
}
