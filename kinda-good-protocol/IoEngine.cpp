#include "IoEngine.h"

kgp::IoEngine::IoEngine(const bool running, QObject *parent)
	: QObject(parent)
	, mSocket(this)
	, mCurrentState(DependancyManager::Instance().GetState())
{
	mSocket.bind(QHostAddress::Any, PORT);
	connect(&mSocket, &QUdpSocket::readyRead, this, &IoEngine::newDataHandler);

	kgp::DependancyManager::Instance().GetLogger().Log("Io Engine initialized");
}

kgp::IoEngine::~IoEngine()
{
	mSocket.close();
	DependancyManager::Instance().GetLogger().Log("Io Engine stopped");
}

void kgp::IoEngine::newDataHandler()
{
	while (mSocket.hasPendingDatagrams())
	{
		// Read in packet
		char buffer[Size::DATA];
		QHostAddress sender;
		quint16 port;
		quint64 sizeRead = mSocket.readDatagram(buffer, Size::DATA, &sender, &port);

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