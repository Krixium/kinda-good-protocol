#pragma once

#include <QtGlobal>

namespace kgp
{
	// Default port
	constexpr short PORT = 8000;

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
		quint64 SequenceNumber;
		quint64 AckNumber;
		quint64 WindowSize;
		size_t DataSize;
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
		// Running state of the thread
		bool running;

		// Current values of the sequence numbers
		quint64 seqNum;
		quint64 ackNum;

		// Size of local receive window
		quint64 rcvWindowSize;

		// Waiting for SYN
		bool idle;
		// Waiting for ACK for SYN
		bool waitSyn;
		// Waiting for incoming data, EOT, or timeout
		bool wait;
		// Waiting for ACK for Data or timeout
		bool dataSent;
		// Has receive timeout been reached
		bool timeoutRcv;
		// Has idle timeout been reached
		bool timeoutIdle;
	};
}
