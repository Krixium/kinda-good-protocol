#include "IoEngine.h"

#include <vector>

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

void kgp::IoEngine::send(const Packet& packet, const QHostAddress& address, const short& port)
{
	{
		mSocket.writeDatagram((char *)&packet, sizeof(packet), address, port);

		std::string receiver = address.toString().toStdString();
		std::string portStr = QString::number(port).toStdString();
		std::string num;
		std::string msg;

		switch (packet.Header.PacketType)
		{
		case PacketType::SYN:
			num = QString::number(packet.Header.SequenceNumber).toStdString();
			msg = "Sending packet " + num + " to " + receiver + " on port " + portStr;
			break;
		case PacketType::ACK:
			num = QString::number(packet.Header.AckNumber).toStdString();
			msg = "ACKing packet " + num + " of " + receiver + " on port " + portStr;
			break;
		case PacketType::DATA:
			num = QString::number(packet.Header.SequenceNumber).toStdString();
			msg = "Sending data packet " + num + " to " + receiver + " on port " + portStr;
			break;
		case PacketType::EOT:
			msg = "Terminating connection with " + receiver + " on port " + portStr;
			break;
		default:
			Q_ASSERT(false);
			break;
		}

		DependancyManager::Instance().Logger().Log(msg);
	}
}

void kgp::IoEngine::sendFrames(std::vector<SlidingWindow::FrameWrapper> list, const QHostAddress& client, const short& port)
{
	for (auto frame : list)
	{
		Packet framePacket;
		memset(&framePacket, 0, sizeof(framePacket));

		framePacket.Header.PacketType = PacketType::DATA;
		framePacket.Header.SequenceNumber = frame.seqNum;
		framePacket.Header.AckNumber = 0;
		framePacket.Header.WindowSize = Size::WINDOW;
		framePacket.Header.DataSize = frame.size;

		memcpy(framePacket.Data, frame.data, frame.size);

		send(framePacket, client, port);
	}
}

void kgp::IoEngine::sendWindow(const QHostAddress& client, const short& port)
{
	if (mWindow.IsEOT())
	{
		sendEot(client, port);
	}
	else
	{
		std::vector<SlidingWindow::FrameWrapper> frames;
		mWindow.GetNextFrames(frames);
		sendFrames(frames, client, port);
		restartRcvTimer();
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
				mClientAddress = sender;
				mClientPort = port;
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
					if (sender == mClientAddress && port == mClientPort)
					{
						// Start sending
						sendWindow(sender, port);
					}
					else
					{
						logInvalidSender(sender, port);
					}
				}
				else
				{
					DependancyManager::Instance().Logger().Error("ACK received for SYN while in invalid state from " + sender.toString().toStdString());
				}
			}
			// If the ACK is for data
			else
			{
				if (mState.DATA_SENT)
				{
					if (sender == mClientAddress && port == mClientPort)
					{
						// If ACK number was valid
						if (mWindow.AckFrame(buffer.Header.AckNumber))
						{
							sendWindow(sender, port);
						}
						else
						{
							DependancyManager::Instance().Logger().Error("Unexpected ACK received(" + QString::number(buffer.Header.AckNumber).toStdString() + ")");
						}
					}
					else
					{
						logInvalidSender(sender, port);
					}
				}
			}
			break;
		case PacketType::DATA:
			if (mState.WAIT)
			{
				// Signal data was read
				emit dataRead(buffer.Data, buffer.Header.DataSize);
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
				// Resend pending frames
				std::vector<SlidingWindow::FrameWrapper> pendingFrames;
				mWindow.GetPendingFrames(pendingFrames);
				sendFrames(pendingFrames, mClientAddress, mClientPort);
				restartRcvTimer();
			}
		}
	}
}