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
		int mSeqNum;
		int mAckNum;
		QTime mRcvTimeout;
		QTime mIdleTimeout;
		SlidingWindow mWindow;

	protected:
		void run();

	public:
		IoEngine(const bool running = true, QObject *parent = nullptr);
		virtual ~IoEngine();

		void Start();
		void Stop();

		void Reset();

		void SendFile(const std::string& filename);

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

		inline void updateTimeout()
		{
			QMutexLocker locker(&mMutex);
			if (QTime::currentTime() > mRcvTimeout) mState.RCV_TO = true;
			if (QTime::currentTime() > mIdleTimeout) mState.IDLE_TO = true;
		}

	private slots:
		void newDataHandler();

	};
}


