#pragma once

#include <QThread>
#include <QUdpSocket>

#include "DependancyManager.h"
#include "res.h"

namespace kgp
{
	class Io : public QThread
	{
	protected:
		bool mIsRunning;
		QUdpSocket mSocket;

		struct State mCurrentState;

	public:
		inline Io(const bool running = true, QObject *parent = nullptr)
			: QThread(parent)
			, mIsRunning(running)
			, mSocket(this)
			, mCurrentState(DependancyManager::Instance().GetState())
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
