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
	mState.idle = true;

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
	start();
}

void kgp::IoEngine::Stop()
{
	DependancyManager::Instance().Logger().Log("Io Engine stopping");
	exit();
}

void kgp::IoEngine::Reset()
{
	DependancyManager::Instance().Logger().Log("Io Engine resetting");
	QMutexLocker locker(&mMutex);
	// Reset state to idle state
	memset(&mState, 0, sizeof(mState));
	mState.rcvWindowSize = Size::WINDOW;
	mState.idle = true;
	// Reset client values
	mClientAddress.clear();
	mClientPort = 0;
	// Reset timeouts
	restartRcvTimer();
	restartIdleTimer();
	// Reset window
	mWindow.Reset();
	// Stop the thread
	Stop();
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
		// Start the thread
		Start();
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
		DependancyManager::Instance().Logger().Log("Transmission finished, sending EOT");
		sendEot(client, port);
		Reset();
	}
	else
	{
		DependancyManager::Instance().Logger().Log("Transmission unfinished, sending data");
		std::vector<SlidingWindow::FrameWrapper> frames;
		mWindow.GetNextFrames(frames);
		sendFrames(frames, client, port);
		restartRcvTimer();
		mState.dataSent = true;
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
				// Transition state
				mClientAddress = datagram.senderAddress();
				mClientPort = datagram.senderPort();
				mState.idle = false;
				mState.wait = true;
				// Start thread
				Start();
				// ACK the SYN
				ackPacket(buffer.Header.SequenceNumber, datagram.senderAddress(), datagram.senderPort());
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
						mState.waitSyn = false;
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
				// Check if it is the incoming packet is a previous packet or next packet
				if (buffer.Header.SequenceNumber <= mState.seqNum)
				{
					DependancyManager::Instance().Logger().Log("Valid packet received");

					// Always ACK a valid packet
					ackPacket(buffer.Header.SequenceNumber, datagram.senderAddress(), datagram.senderPort());

					// If it was new data
					if (buffer.Header.SequenceNumber == mState.seqNum)
					{
						// Signal new data was read
						emit dataRead(buffer.Data, buffer.Header.DataSize);
						// Increment sequence number counter
						mState.seqNum += buffer.Header.DataSize;
					}
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
				DependancyManager::Instance().Logger().Log("EOT received, resetting state");
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
	// Only run when there is a connection and not idle
	while (!mState.idle)
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

			// If syn timed out
			if (mState.waitSyn)
			{
				// Just reset
				Reset();
			}
			// If data packet timed out
			else if (mState.dataSent)
			{
				// Resend pending frames
				DependancyManager::Instance().Logger().Log("Resending pending packets");
				std::vector<SlidingWindow::FrameWrapper> pendingFrames;
				mWindow.GetPendingFrames(pendingFrames);
				sendFrames(pendingFrames, mClientAddress, mClientPort);
				restartRcvTimer();
				restartIdleTimer();
			}
			// If ACKs timed out
			else if (mState.wait)
			{
				DependancyManager::Instance().Logger().Log("Resending ACK");
				// Resend all ACKs
				ackPacket(mState.seqNum, mClientAddress, mClientPort);
				restartRcvTimer();
				restartIdleTimer();
			}
			else
			{
				// This should never happen
				DependancyManager::Instance().Logger().Error("Receive timeout reached while in invalid state.");
			}
		}
	}
}