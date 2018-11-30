/*---------------------------------------------------------------------------------------
-- SOURCE FILE:             Logger.h
--
-- PROGRAM:                 KindaGoodProtocol
--
-- FUNCTIONS:               N/A
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNERS:               Benny Wang
--
-- PROGRAMMERS:             Benny Wang
--
-- NOTES:
--                          This is a class that handles logging all messages passed to it
--                          to the correct places.
---------------------------------------------------------------------------------------*/
#pragma once

#include <memory>
#include <string>
#include <fstream>
#include <ctime>

#ifdef _WIN32
#include <sys/utime.h>
#else 
#include <utime.h>
#endif

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QHostAddress>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>

#include "res.h"

namespace kgp
{
    class Logger
    {
    private:
        QFile mLogFile;
        QMutex mMutex;

    public:
        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::Logger::Logger
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               kgp::Logger::Logger()
        --
        -- NOTES:
        --                          Constructor for Logger. Creates and opens the log file for writing.
        --------------------------------------------------------------------------------------------------*/
        inline Logger()
            : mLogFile(LOG_FILE)
        {

            if (mLogFile.isOpen()) mLogFile.close();
            mLogFile.open(QIODevice::WriteOnly);
        }

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::Logger::~Logger
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               kgp::Logger::~Logger()
        --
        -- NOTES:
        --                          Deconstructor for Logger. Closes the logging file.
        --------------------------------------------------------------------------------------------------*/
        inline ~Logger()
        {
            if (mLogFile.isOpen()) mLogFile.close();
        }

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::Logger::Log
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::Logger::Log(const std::string& msg)
        --                              msg: The message to log.
        --
        -- NOTES:
        --                          Logs msg with severity "Log" and the timestamp.
        --------------------------------------------------------------------------------------------------*/
        inline void Log(const std::string& msg)
        {
            QString line("[ " + QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss") + " Log ]: " + msg.c_str());
            emit write(line);
        }

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::Logger::Error
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::Logger::Error(const std::string& msg)
        --                              msg: The message to log.
        --
        -- NOTES:
        --                          Logs msg with severity "Error" and the timestamp.
        --------------------------------------------------------------------------------------------------*/
        inline void Error(const std::string& msg)
        {
            QString line("[ " + QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss") + " Error ]: " + msg.c_str());
            emit write(line);
        }

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::Logger::LogPacket
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:                void kgp::Logger::LogPacket(const Packet& packet, const QHostAddress& sender)
        --                              packet: The packet to log.
        --                              sender: The sender of the packet.
        --
        -- NOTES:
        --                           Formats and logs a packet with severity "Log" and the timestamp.
        --------------------------------------------------------------------------------------------------*/
        inline void LogPacket(const Packet& packet, const QHostAddress& sender)
        {
            std::string address(sender.toString().toStdString());
            std::string packetType(QString::number((int)packet.Header.PacketType).toStdString());
            std::string ackNum(QString::number(packet.Header.AckNumber).toStdString());
            std::string seqNum(QString::number(packet.Header.SequenceNumber).toStdString());
            std::string windowSize(QString::number(packet.Header.WindowSize).toStdString());
            std::string dataSize(QString::number(packet.Header.DataSize).toStdString());
            std::string data(QString(packet.Data).toStdString());
            Log("Address: " + address + "        Packet Type: " + packetType);
            Log("ACK #: " + ackNum + "            Sequence #: " + seqNum);
            Log("Data Size: " + dataSize + "    Window Size: " + windowSize);
            Log("\tData: " + data);
        }

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::Logger::LogInvalidSender
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::Logger::LogInvalidSender(const QHostAddress& expectedIp, const short& expectedPort, const QHostAddress& actualIp, const short& actualPort)
        --                              expectedIp: The expected IP of the sender.
        --                              expectedPort: The expected port of the sender.
        --                              actualIp: The actual IP of the sender.
        --                              actualPort: The actual port of the sender.
        --
        -- NOTES:
        --                          Logs information about about a sender when data from an invalid host is
        --                          received. The message is logged with severity "Error".
        --------------------------------------------------------------------------------------------------*/
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

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::Logger::updateTimestamp
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                William Murphy
        --
        -- PROGRAMMER:              William Murphy
        --
        -- INTERFACE:               bool kgp::Logger::updateTimestamp(const std::string& filename)
        --                              filename: The name of the file whose timestamp will be updated.
        --
        -- NOTES:					This function is required for QFileSystemWatcher to be properly notified
		--							of file updates, at least on Windows.
		--							
        --------------------------------------------------------------------------------------------------*/
		inline bool updateTimestamp(const std::string& filename) 
		{
			struct stat fstat;
			struct utimbuf new_time;

			if (stat(kgp::LOG_FILE, &fstat) != 0)
			{
				return false;
			}

			new_time.actime = fstat.st_atime;
			new_time.modtime = time(NULL);

			if (utime(kgp::LOG_FILE, &new_time)) {
				return false;
			}
			return true;
		}

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::Logger::write
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::Logger::write(const std::string& line)
        --                              data: The data to write.
        --
        -- NOTES:
        --                          Writes data to the required areas. Currently will output the data to
        --                          stdout and write it to the log file.
        --------------------------------------------------------------------------------------------------*/
        inline void write(QString& data)
        {
            QMutexLocker locker(&mMutex);
            qDebug() << data;
            mLogFile.write(data.toStdString().c_str());
            mLogFile.write("\n");
			// Flushes the stream so that logs can be displayed by the log viewer widget
			mLogFile.flush();
			updateTimestamp(kgp::LOG_FILE);
			// Update the file timestamp
        }
    };
}
