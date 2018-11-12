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
		QTime mRcvTimeout;
		QTime mIdleTimeout;
		SlidingWindow mWindow;

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

	private:
		inline void startRcvTimeout()
		{
			QMutexLocker locker(&mMutex);
			mRcvTimeout = QTime::currentTime().addMSecs(Timeout::RCV);
		}

		inline void startIdleTimeout()
		{
			QMutexLocker locker(&mMutex);
			mIdleTimeout = QTime::currentTime().addMSecs(Timeout::IDLE);
		}

		inline void clearRcvTimeout()
		{
			QMutexLocker locker(&mMutex);
			mRcvTimeout = QTime::currentTime().addSecs(1000000);
			mState.RCV_TO = false;
		}

		inline void clearIdleTimeout()
		{
			QMutexLocker locker(&mMutex);
			mIdleTimeout = QTime::currentTime().addSecs(1000000);
			mState.IDLE_TO = false;
		}

		inline void updateTimeout()
		{
			QMutexLocker locker(&mMutex);
			if (QTime::currentTime() > mRcvTimeout) mState.RCV_TO = true;
			if (QTime::currentTime() > mIdleTimeout) mState.IDLE_TO = true;
		}

		inline void createSynPacket(Packet *buffer)
		{
			memset(buffer, 0, sizeof(buffer));
			buffer->Header.AckNumber = 0;
			buffer->Header.SequenceNumber = 0;
			buffer->Header.WindowSize = Size::WINDOW;
			buffer->Header.PacketType = PacketType::SYN;
		}

		inline void send(const Packet& packet, const QHostAddress& address, const short& port)
		{
			mSocket.writeDatagram((char *)&packet, sizeof(packet), address, port);
		}

		inline void ackPacket(const Packet& incoming, const QHostAddress& sender, const short& port)
		{
			Packet res;
			memset(&res, 0, sizeof(res));

			res.Header.AckNumber = incoming.Header.SequenceNumber;
			res.Header.SequenceNumber = 0;
			res.Header.WindowSize = Size::WINDOW;
			res.Header.PacketType = PacketType::ACK;

			send(res, sender, port);
		}

	private slots:
		void newDataHandler();

	};
}


