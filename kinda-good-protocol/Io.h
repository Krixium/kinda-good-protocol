#pragma once

#include "res.h"

#include <QThread>
#include <QUdpSocket>

namespace kgp
{
	class Io : public QThread
	{
	protected:
		bool mIsRunning;
		QUdpSocket mSocket;

		struct State mState;

	public:
		inline Io(const bool running = true, QObject *parent = nullptr)
			: QThread(parent)
			, mIsRunning(running)
			, mSocket(this)
			, mState(DependancyManager::Instance().GetState())
		{
		}

		virtual ~Io()
		{
			mSocket.close();
		}

		void Start() { start(); }
		void Stop() { mIsRunning = false; }

	protected:
		void run() = 0;
	};
}
