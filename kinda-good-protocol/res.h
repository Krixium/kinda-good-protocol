#pragma once

#include <string>

#include <QDebug>
#include <QDateTime>

namespace kgp
{
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

	void inline Log(const std::string& msg) 
	{
		qDebug() << "[ " + QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss") + " KGP ]: " + msg.c_str();
		// TODO: Write to file here
		// TODO: Refactor to a class to handle logging
	}
}
