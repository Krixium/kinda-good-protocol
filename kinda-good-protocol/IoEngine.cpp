#include "IoEngine.h"

kgp::IoEngine::IoEngine(const bool running, QObject *parent)
	: QThread(parent)
	, mRunning(false)
	, mSocket(this)
	, mState(DependancyManager::Instance().GetState())
	, mSeqNum(0)
	, mAckNum(0)
	, mRcvTimer()
	, mIdleTimer()
	, mClientAddress(QHostAddress())
	, mClientPort(0)
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
	mClientAddress = QHostAddress();
	mClientPort = 0;
	mSocket.bind(QHostAddress::Any, PORT);
	// Reset timeouts
	restartRcvTimer();
	restartIdleTimer();
	// Reset window
	mWindow.Reset();
}

bool kgp::IoEngine::StartFileSend(const std::string& filename, const std::string& address, const short& port)
{
	QMutexLocker locker(&mMutex);

	// If not already sending
	if (!mState.DATA_SENT)
	{
		DependancyManager::Instance().Logger().Log("Sending file " + filename + " to " + address);
		// Buffer file
		QFile file(filename.c_str());
		mWindow.BufferFile(file);
		// Set client
		mClientAddress = QHostAddress(address.c_str());
		mClientPort = port;
		// Send SYN packet
		Packet synPacket;
		createSynPacket(&synPacket);
		send(synPacket, mClientAddress, mClientPort);
		// Start timeouts
		restartRcvTimer();
		restartIdleTimer();
		// Set state
		mState.IDLE = false;
		mState.WAIT_SYN = true;
		return true;
	}
	else
	{
		DependancyManager::Instance().Logger().Error("Already sending");
		return false;
	}
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

		// Restart idle timeout
		restartIdleTimer();

		// Handle packet accordingly
		switch (buffer.Header.PacketType)
		{
		case PacketType::SYN:
			if (mState.IDLE)
			{
				// ACK the SYN
				ackPacket(buffer, sender, port);

				// Transition state
				mMutex.lock();
				mState.IDLE = false;
				mState.WAIT = true;
				mMutex.unlock();
			}
			else
			{
				DependancyManager::Instance().Logger().Error("SYN received while in invalid state from " + sender.toString().toStdString());
			}
			break;
		case PacketType::ACK:
			// If the ACK is for a SYN
			if (buffer.Header.AckNumber == 0)
			{
				if (mState.WAIT_SYN)
				{
					// Start sending

					// TODO: implement

					restartRcvTimer();
				}
			}
			// If the ACK is for data
			else
			{
				if (mState.DATA_SENT)
				{
					// Continue sending

					// TODO: implement

					restartRcvTimer();
				}
			}
			break;
		case PacketType::DATA:
			if (mState.WAIT)
			{
				// Signal data was read
				emit dataRead(buffer.Data, sizeof(buffer.Data));
				// Log the packet
				logDataPacket(buffer, sender);
				// ACK the packet
				ackPacket(buffer, sender, port);
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
		checkTimers();

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

			// Reset if SYN timed out, otherwise resend all packets
			if (mState.WAIT_SYN)
			{
				Reset();
			}
			else
			{
				// TODO: Resend all packets and restart receive timer
			}
		}
	}
}