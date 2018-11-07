#include "res.h"

#include <QHostAddress>

#include "DependancyManager.h"
#include "Receiver.h"

kgp::Receiver::Receiver(const bool running, QObject *parent)
	: Io(running, parent)
{
	mSocket.bind(QHostAddress::Any, PORT);
	connect(&mSocket, &QUdpSocket::readyRead, this, &Receiver::newDataHandler);

	DependancyManager::Instance().GetLogger().Log("Receiver thread initialized");
}

void kgp::Receiver::run()
{
	DependancyManager::Instance().GetLogger().Log("Receiver thread started");
	while (mIsRunning)
	{

	}

	DependancyManager::Instance().GetLogger().Log("Receiver thread finished");
	quit();
}

void kgp::Receiver::newDataHandler()
{
	while (mSocket.hasPendingDatagrams())
	{
		// Read in packet
		char buffer[size::DATA];
		QHostAddress sender;
		quint16 port;
		quint64 sizeRead = mSocket.readDatagram(buffer, size::DATA, &sender, &port);

		// Deal with previous state first
		// If we are in the data received state
		if (mCurrentState.DATA_RCV)
		{

		}

		// If we are in the data sent state
		if (mCurrentState.DATA_SND) 
		{

		}

		// Process packet to set next state
	}
}
