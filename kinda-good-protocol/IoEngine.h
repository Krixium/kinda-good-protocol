#pragma once

#include <string>

#include <QHostAddress>
#include <QMutex>
#include <QMutexLocker>
#include <QUdpSocket>
#include <QThread>
#include <QTime>

#include "DependancyManager.h"
#include "res.h"
#include "SlidingWindow.h"

namespace kgp
{
	class IoEngine : public QThread
	{
		Q_OBJECT

	private:
		QUdpSocket mSocket;
		QMutex mMutex;

		bool mRunning;
		struct State mState;
		// should we random this number before incrementing
		int mSeqNum;
		// how should this number work
		int mAckNum;
		QTime mRcvTimer;
		QTime mIdleTimer;
		SlidingWindow mWindow;
		quint64 mRcvWindowSize;

		QHostAddress mClientAddress;
		short mClientPort;

	protected:
		void run();

	public:
		IoEngine(const bool running = true, QObject *parent = nullptr);
		virtual ~IoEngine();

		void Start();
		void Stop();

		void Reset();

		bool StartFileSend(const std::string& filename, const std::string& address, const short& port);

		void SetReceiveWindowSize(quint64 size) { mRcvWindowSize = size; }

	private:
		inline void logDataPacket(const Packet& packet, const QHostAddress& sender)
		{
			std::string address(sender.toString().toStdString());
			std::string seqNum(QString::number(packet.Header.SequenceNumber).toStdString());
			std::string data(QString(packet.Data).toStdString());
			DependancyManager::Instance().Logger().Log("Sender: " + address + ", Sequence number: " + seqNum);
			DependancyManager::Instance().Logger().Log("\tData: " + data);
		}

		inline void logInvalidSender(const QHostAddress& sender, const short& port)
		{
			std::string expectedClient = mClientAddress.toString().toStdString();
			std::string actualClient = sender.toString().toStdString();
			std::string expectedPort = QString::number(port).toStdString();
			std::string actualPort = QString::number(port).toStdString();
			DependancyManager::Instance().Logger().Error("Expected Client: " + expectedClient + ", port: " + expectedPort);
			DependancyManager::Instance().Logger().Error("Received Client: " + actualClient + ", port: " + actualPort);
		}

		inline void restartRcvTimer()
		{
			QMutexLocker locker(&mMutex);
			mRcvTimer.start();
			mState.RCV_TO = false;
		}

		inline void restartIdleTimer()
		{
			QMutexLocker locker(&mMutex);
			mIdleTimer.start();
			mState.IDLE_TO = false;
		}

		inline void checkTimers()
		{
			QMutexLocker locker(&mMutex);
			if (mRcvTimer.elapsed() > Timeout::RCV) mState.RCV_TO = true;
			if (mIdleTimer.elapsed() > Timeout::IDLE) mState.IDLE_TO = true;
		}

		inline void createSynPacket(Packet *buffer)
		{
			memset(buffer, 0, sizeof(buffer));
			buffer->Header.AckNumber = 0;
			buffer->Header.SequenceNumber = 0;
			buffer->Header.WindowSize = mRcvWindowSize;
			buffer->Header.PacketType = PacketType::SYN;
			buffer->Header.DataSize = 0;
		}

		inline void ackPacket(const Packet& incoming, const QHostAddress& sender, const short& port)
		{
			Packet res;
			memset(&res, 0, sizeof(res));
			res.Header.AckNumber = incoming.Header.SequenceNumber;
			res.Header.SequenceNumber = 0;
			res.Header.WindowSize = mRcvWindowSize;
			res.Header.PacketType = PacketType::ACK;
			res.Header.DataSize = 0;

			send(res, sender, port);
		}

		inline void sendEot(const QHostAddress& receiver, const short& port)
		{
			Packet res;
			memset(&res, 0, sizeof(res));
			res.Header.PacketType = PacketType::EOT;
			res.Header.SequenceNumber = 0;
			res.Header.AckNumber = 0;
			res.Header.WindowSize = 0;
			res.Header.DataSize = 0;
			send(res, receiver, port);
		}

		void send(const Packet& packet, const QHostAddress& address, const short& port);
		void sendFrames(std::vector<SlidingWindow::FrameWrapper> list, const QHostAddress& client, const short& port);
		void sendWindow(const QHostAddress& client, const short& port);

	private slots:
		void newDataHandler();

	signals:
		void dataRead(const char *data, const size_t& size);

	};
}


