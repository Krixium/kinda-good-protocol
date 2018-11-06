#pragma once

namespace kgp
{
	// Control characters
	constexpr char DATA = 0x02;
	constexpr char ACK = 0x05;
	constexpr char EOT = 0x04;
	constexpr char SYN = 0x15;

	// Sizes
	namespace size
	{
		constexpr size_t HEADER = sizeof(PacketHeader);
		constexpr size_t PACKET = 1500;
	}

	// Packet header
	struct PacketHeader
	{
		char PacketType;
		unsigned int SequenceNumber;
		unsigned int AckNumber;
		unsigned int WindowSize;
	};

	// Packet
	struct Packet
	{
		struct PacketHeader Header;
		char Data[size::HEADER - size::PACKET];
	};
}
