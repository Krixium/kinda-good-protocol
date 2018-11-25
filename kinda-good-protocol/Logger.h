#pragma once

#include <memory>
#include <string>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QHostAddress>

#include "res.h"

namespace kgp
{
	class Logger
	{
	public:
		QFile mLogFile;

	public:
		inline Logger()
			: mLogFile(LOG_FILE)
		{
			if (mLogFile.isOpen()) mLogFile.close();
			mLogFile.open(QIODevice::Append | QIODevice::Text);
		}

		inline ~Logger()
		{
			if (mLogFile.isOpen()) mLogFile.close();
		}

		inline void Log(const std::string& msg)
		{
			QString line("[ " + QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss") + " Log ]: " + msg.c_str());
			write(line);
		}

		inline void Error(const std::string& msg)
		{
			QString line("[ " + QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss") + " Error ]: " + msg.c_str());
			write(line);
		}

		inline void LogDataPacket(const Packet& packet, const QHostAddress& sender)
		{
			std::string address(sender.toString().toStdString());
			std::string packetType(QString::number((int)packet.Header.PacketType).toStdString());
			std::string ackNum(QString::number(packet.Header.AckNumber).toStdString());
			std::string seqNum(QString::number(packet.Header.SequenceNumber).toStdString());
			std::string windowSize(QString::number(packet.Header.WindowSize).toStdString());
			std::string dataSize(QString::number(packet.Header.DataSize).toStdString());
			std::string data(QString(packet.Data).toStdString());
			Log("Address: " + address + "		Packet Type: " + packetType);
			Log("ACK #: " + ackNum + "			Sequence #: " + seqNum);
			Log("Data Size: " + dataSize + "	Window Size: " + windowSize);
			Log("\tData: " + data);
		}

		inline void LogInvalidSender(const QHostAddress& expectedIp, const short& expectedPort, const QHostAddress& actualIp, const short& actualPort)
		{
			std::string strExpectedClient = expectedIp.toString().toStdString();
			std::string strActualClient = actualIp.toString().toStdString();
			std::string strExpectedPort = QString::number(expectedPort).toStdString();
			std::string strActualPort = QString::number(actualPort).toStdString();
			Error("Expected Client: " + strExpectedClient + ", port: " + strExpectedPort);
			Error("Received Client: " + strActualClient + ", port: " + strActualPort);
		}


	private:
		inline void write(QString line)
		{
			qDebug() << line;
			mLogFile.write(line.toStdString().c_str() + '\n');
		}
	};
}
