#include "IoEngine.h"

#include <vector>

#include <QNetworkDatagram>

kgp::IoEngine::IoEngine(const bool running, QObject *parent)
	: QThread(parent)
	, mSocket(this)
	, mState()
	, mRcvTimer()
	, mIdleTimer()
	, mClientAddress()
	, mClientPort(0)
{
	memset(&mState, 0, sizeof(mState));
	mState.rcvWindowSize = Size::WINDOW;

	mSocket.bind(QHostAddress::Any, PORT);
	connect(&mSocket, &QUdpSocket::readyRead, this, &IoEngine::newDataHandler);

	kgp::DependancyManager::Instance().Logger().Log("Io Engine initialized");
}

kgp::IoEngine::~IoEngine()
{
	DependancyManager::Instance().Logger().Log("Io Engine stopped");
	mSocket.close();
}

void kgp::IoEngine::Start()
{
	DependancyManager::Instance().Logger().Log("Io Engine starting");
	QMutexLocker locker(&mMutex);
	mState.running = true;
	start();
}

void kgp::IoEngine::Stop()
{
	DependancyManager::Instance().Logger().Log("Io Engine stopping");
	QMutexLocker locker(&mMutex);
	mState.running = false;
	exit();
}

void kgp::IoEngine::Reset()
{
	DependancyManager::Instance().Logger().Log("Io Engine resetting");
	QMutexLocker locker(&mMutex);

	// Reset state to idle state
	memset(&mState, 0, sizeof(mState));
	mState.running = true;
	mState.rcvWindowSize = Size::WINDOW;
	mState.idle = true;
	// Reset socket
	mClientAddress.clear();
	mClientPort = 0;
	mSocket.reset();
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
	if (!mState.dataSent)
	{
		DependancyManager::Instance().Logger().Log("Sending file " + filename + " to " + address);
		// Buffer file
		QFile file(filename.c_str());
		// Return false if the file could not be read
		if (!mWindow.BufferFile(file)) return false;
		// Set client
		mClientAddress.setAddress(address.c_str());
		mClientPort = port;
		// Send SYN packet
		Packet synPacket;
		createSynPacket(&synPacket);
		send(synPacket, mClientAddress, mClientPort);
		// Start timeouts
		restartRcvTimer();
		restartIdleTimer();
		// Set state
		mState.idle = false;
		mState.waitSyn = true;
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
	mSocket.writeDatagram((char *)&packet, sizeof(packet), address, port);
	DependancyManager::Instance().Logger().Log("Sending packet ...");
	DependancyManager::Instance().Logger().LogDataPacket(packet, address);
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
		framePacket.Header.WindowSize = mState.rcvWindowSize;
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
		//quint64 sizeRead = mSocket.readDatagram((char *)&buffer, Size::DATA, &sender, &port);
		QNetworkDatagram datagram = mSocket.receiveDatagram();
		

		// If less than a header was read print error and continue
		if (datagram.data().size() < sizeof(struct PacketHeader))
		{
			DependancyManager::Instance().Logger().Error("Not enough data was read from " + datagram.senderAddress().toString().toStdString());
			continue;
		}

		memcpy(&buffer, datagram.data().data(), datagram.data().size());
		
		// Restart idle timeout
		restartIdleTimer();

		// Log receive packet here
		DependancyManager::Instance().Logger().Log("Receiving packet ...");
		DependancyManager::Instance().Logger().LogDataPacket(buffer, datagram.senderAddress());

		// Handle packet accordingly
		switch (buffer.Header.PacketType)
		{
		case PacketType::SYN:
			if (mState.idle)
			{
				// ACK the SYN
				ackPacket(buffer, datagram.senderAddress(), datagram.senderPort());

				// Transition state
				mMutex.lock();
				mClientAddress = datagram.senderAddress();
				mClientPort = datagram.senderPort();
				mState.idle = false;
				mState.wait = true;
				mMutex.unlock();
			}
			else
			{
				DependancyManager::Instance().Logger().Error("SYN received while in invalid state from " + datagram.senderAddress().toString().toStdString());
			}
			break;
		case PacketType::ACK:
			if (datagram.senderAddress().toIPv4Address() == mClientAddress.toIPv4Address() && datagram.senderPort() == mClientPort)
			{
				// Adjust window size
				mWindow.SetWindowSize(buffer.Header.WindowSize);

				// If the ACK is for a SYN
				if (buffer.Header.AckNumber == 0)
				{
					if (mState.waitSyn)
					{
						sendWindow(datagram.senderAddress(), datagram.senderPort());
					}
					else
					{
						DependancyManager::Instance().Logger().Error("ACK received for SYN while in invalid state from " + datagram.senderAddress().toString().toStdString());
					}
				}
				// If the ACK is for data
				else
				{
					if (mState.dataSent)
					{
						// If ACK number was valid
						if (mWindow.AckFrame(buffer.Header.AckNumber))
						{
							sendWindow(datagram.senderAddress(), datagram.senderPort());
						}
						else
						{
							DependancyManager::Instance().Logger().Error("Unexpected ACK received(" + QString::number(buffer.Header.AckNumber).toStdString() + ")");
						}

					}
				}
			}
			else
			{
				DependancyManager::Instance().Logger().LogInvalidSender(mClientAddress, mClientPort, datagram.senderAddress(), datagram.senderPort());
			}
			break;
		case PacketType::DATA:
			if (mState.wait)
			{
				// Check if it was the correct packet
				if (buffer.Header.SequenceNumber == mState.seqNum)
				{
					// Signal data was read
					emit dataRead(buffer.Data, buffer.Header.DataSize);

					// Increment sequence number counter
					mState.seqNum += buffer.Header.DataSize;

					// ACK the packet
					ackPacket(buffer, datagram.senderAddress(), datagram.senderPort());

					DependancyManager::Instance().Logger().Log("Valid packet received");
				}
				else
				{
					DependancyManager::Instance().Logger().Error("Invalid packet received, expecting sequence number " + QString::number(mState.seqNum).toStdString());
				}
			}
			else
			{
				DependancyManager::Instance().Logger().Error("Data received while in invalid state from " + datagram.senderAddress().toString().toStdString());
			}
			break;
		case PacketType::EOT:
			if (mState.wait)
			{
				// Valid EOT was received so reset
				Reset();
			}
			else
			{
				DependancyManager::Instance().Logger().Error("EOT received while in invalid state from " + datagram.senderAddress().toString().toStdString());
			}
			break;
		}
	}
}

void kgp::IoEngine::run()
{
	mState.idle = true;
	while (mState.running)
	{
		checkTimers();

		// If idle timeout has been reached
		if (mState.timeoutIdle)
		{
			DependancyManager::Instance().Logger().Log("Idle timeout reached");
			Reset();
		}

		// If receive timeout has been reached
		if (mState.timeoutRcv)
		{
			DependancyManager::Instance().Logger().Log("Receive timeout reached");

			// Reset if SYN timed out, otherwise resend all packets
			if (mState.waitSyn)
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