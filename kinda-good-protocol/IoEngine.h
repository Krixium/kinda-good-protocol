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
		QMutex mMutex;

		struct State mState;
		
		QUdpSocket mSocket;
		QHostAddress mClientAddress;
		short mClientPort;

		QTime mRcvTimer;
		QTime mIdleTimer;
		SlidingWindow mWindow;

	protected:
		void run();

	public:
		IoEngine(const bool running = true, QObject *parent = nullptr);
		virtual ~IoEngine();

		void Start();
		void Stop();

		void Reset();

		bool StartFileSend(const std::string& filename, const std::string& address, const short& port);

		void SetReceiveWindowSize(quint64 size) { mState.rcvWindowSize = size; }

	private:
		inline void restartRcvTimer()
		{
			mRcvTimer.start();
			mState.timeoutRcv = false;
		}

		inline void restartIdleTimer()
		{
			mIdleTimer.start();
			mState.timeoutIdle = false;
		}

		inline void checkTimers()
		{
			QMutexLocker locker(&mMutex);
			if (mRcvTimer.elapsed() > Timeout::RCV) mState.timeoutRcv = true;
			if (mIdleTimer.elapsed() > Timeout::IDLE) mState.timeoutIdle = true;
		}

		inline void createSynPacket(Packet *buffer)
		{
			memset(buffer, 0, sizeof(buffer));
			buffer->Header.AckNumber = 0;
			buffer->Header.SequenceNumber = 0;
			buffer->Header.WindowSize = mState.rcvWindowSize;
			buffer->Header.PacketType = PacketType::SYN;
			buffer->Header.DataSize = 0;
		}

		inline void ackPacket(const Packet& incoming, const QHostAddress& sender, const short& port)
		{
			Packet res;
			memset(&res, 0, sizeof(res));
			res.Header.AckNumber = incoming.Header.SequenceNumber;
			res.Header.SequenceNumber = 0;
			res.Header.WindowSize = mState.rcvWindowSize;
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


