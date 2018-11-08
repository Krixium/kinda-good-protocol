#pragma once

#include <QHostAddress>
#include <QUdpSocket>

#include "DependancyManager.h"
#include "res.h"

namespace kgp
{
	class IoEngine : public QObject
	{
		Q_OBJECT

	private:
		QUdpSocket mSocket;

		struct State mCurrentState;

	public:
		IoEngine(const bool running = true, QObject *parent = nullptr);
		virtual ~IoEngine();

	private slots:
		void newDataHandler();

	};
}


