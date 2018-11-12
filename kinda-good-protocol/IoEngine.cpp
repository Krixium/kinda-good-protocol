#include "IoEngine.h"

kgp::IoEngine::IoEngine(const bool running, QObject *parent)
	: QThread(parent)
	, mRunning(false)
	, mSocket(this)
	, mState(DependancyManager::Instance().GetState())
	, mSeqNum(0)
	, mAckNum(0)
	, mRcvTimeout(QTime::currentTime().addSecs(1000000))
	, mIdleTimeout(QTime::currentTime().addSecs(1000000))
{
	mSocket.bind(QHostAddress::Any, PORT);
	connect(&mSocket, &QUdpSocket::readyRead, this, &IoEngine::newDataHandler);

	kgp::DependancyManager::Instance().Logger().Log("Io Engine initialized");
}

kgp::IoEngine::~IoEngine()
{
	mSocket.close();
	DependancyManager::Instance().Logger().Log("Io Engine stopped");
}

void kgp::IoEngine::Start()
{
	DependancyManager::Instance().Logger().Log("Io Engine starting");
	mRunning = true;
	start();
}

void kgp::IoEngine::Stop()
{
	DependancyManager::Instance().Logger().Log("Io Engine stopping");
	QMutexLocker locker(&mMutex);
	mRunning = false;
}

void kgp::IoEngine::Reset()
{
	DependancyManager::Instance().Logger().Log("Io Engine resetting");
	QMutexLocker locker(&mMutex);

	// Reset state
	memset(&mState, 0, sizeof(mState));
	mState.IDLE = true;
	mSeqNum = 0;
	mAckNum = 0;
	// Reset socket
	mSocket.close();
	mSocket.bind(QHostAddress::Any, PORT);
	// Reset timeouts
	mRcvTimeout = QTime::currentTime().addSecs(1000000);
	mIdleTimeout = QTime::currentTime().addSecs(1000000);
	// Reset window
	mWindow.Reset();
}

void kgp::IoEngine::SendFile(const std::string& filename)
{
	DependancyManager::Instance().Logger().Log("Sending file " + filename);
	QMutexLocker locker(&mMutex);

	// Buffer file
	QFile file(filename.c_str());
	mWindow.BufferFile(file);

	// Send SYN packet

	// Set state
	mState.WAIT_SYN = true;
}

void kgp::IoEngine::newDataHandler()
{
	while (mSocket.hasPendingDatagrams())
	{
		// Read in packet
		Packet buffer;
		QHostAddress sender;
		quint16 port;
		quint64 sizeRead = mSocket.readDatagram((char *)&buffer, Size::DATA, &sender, &port);

		// If less than a header was read print error and continue
		if (sizeRead < sizeof(struct PacketHeader))
		{
			DependancyManager::Instance().Logger().Error("Not enough data was read from " + sender.toString().toStdString());
			continue;
		}

		// Not idle so extend timeout
		startIdleTimeout();

		// Handle packet accordingly
		switch (buffer.Header.PacketType)
		{
		case PacketType::SYN:
			if (mState.IDLE)
			{
				// Valid SYN was received, handle it	
			}
			else
			{
				DependancyManager::Instance().Logger().Error("SYN received while in invalid state from " + sender.toString().toStdString());
			}
			break;
		case PacketType::ACK:
			if (mState.WAIT_SYN)
			{
				// Valid ACK received for sent SYN, handle it
			}
			else if (mState.DATA_SENT)
			{
				// Valid ACK received for sent data, handle it
			}
			else
			{
				DependancyManager::Instance().Logger().Error("ACK received while in invalid state from " + sender.toString().toStdString());
			}
			break;
		case PacketType::DATA:
			if (mState.WAIT)
			{
				// Valid Data was received, handle it
			}
			else
			{
				DependancyManager::Instance().Logger().Error("Data received while in invalid state from " + sender.toString().toStdString());
			}
			break;
		case PacketType::EOT:
			if (mState.WAIT)
			{
				// Valid EOT was received so reset
				Reset();
			}
			else
			{
				DependancyManager::Instance().Logger().Error("EOT received while in invalid state from " + sender.toString().toStdString());
			}
			break;
		}
	}
}

void kgp::IoEngine::run()
{
	while (mRunning)
	{
		updateTimeout();

		// If idle timeout has been reached
		if (mState.IDLE_TO)
		{
			DependancyManager::Instance().Logger().Log("Idle timeout reached");
			Reset();
		}

		// If receive timeout has been reached
		if (mState.RCV_TO) 
		{
			DependancyManager::Instance().Logger().Log("Receive timeout reached");

			// Resend last packet

			mState.RCV_TO = false;
		}
	}
}