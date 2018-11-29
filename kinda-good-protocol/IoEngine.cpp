/*---------------------------------------------------------------------------------------
-- SOURCE FILE:             IoEngine.cpp
--
-- PROGRAM:                 KindaGoodProtocol
--
-- FUNCTIONS:               N/A
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNERS:               Benny Wang
--
-- PROGRAMMERS:             Benny Wang
--
-- NOTES:
--                          The engine of the application that handles all protocol logic.
---------------------------------------------------------------------------------------*/
#include "IoEngine.h"

#include <vector>

#include <QNetworkDatagram>

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::IoEngine
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               kgp::IoEngine::IoEngine(const bool running, QObject *parent)
--                              parent: The parent QObject.
--
-- NOTES:
--                          Constructor for the IoEngine. Creates a sliding window and binds a port.
--------------------------------------------------------------------------------------------------*/
kgp::IoEngine::IoEngine(QObject *parent)
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

    kgp::DependencyManager::Instance().Logger().Log("Io Engine initialized");
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::~IoEngine
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               kgp::IoEngine::~IoEngine()
--
-- NOTES:
--                          Deconstructor for the IoEngine. Closes the bound socket.
--------------------------------------------------------------------------------------------------*/
kgp::IoEngine::~IoEngine()
{
    DependencyManager::Instance().Logger().Log("Io Engine stopped");
    mSocket.close();
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::Start
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void kgp::IoEngine::Start()
--
-- NOTES:
--                          Wrapper function of QThread::start. Starts the thread and executes the
--                          overloaded QThread::run function.
--------------------------------------------------------------------------------------------------*/
void kgp::IoEngine::Start()
{
    DependencyManager::Instance().Logger().Log("Io Engine starting");
    start();
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::Stop
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void kgp::IoEngine::Stop()
--
-- NOTES:
--                          Stops the thread of the IoEngine.
--------------------------------------------------------------------------------------------------*/
void kgp::IoEngine::Stop()
{
    DependencyManager::Instance().Logger().Log("Io Engine stopping");
    exit();
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::Reset
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void kgp::IoEngine::Reset()
--
-- NOTES:
--                          Resets the state of the IoEngine to the default state where the socket
--                          is still bond and there is not connection established yet.
--------------------------------------------------------------------------------------------------*/
void kgp::IoEngine::Reset()
{
    DependencyManager::Instance().Logger().Log("Io Engine resetting");
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

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::StartFileSend
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               bool kgp::IoEngine::StartFileSend(const std::string& filename, const std::string& address, const short& port)
--                              filename: The name of the file to send.
--                              address: The address to send to.
--                              port: The port to send the file on.
--
-- RETURN:                  True if sending has started, false otherwise.
--
-- NOTES:
--                          Initiates sending. Attempts to open the given file and buffers it in
--                          the sliding window. Then sends a syn packet and transitions to the waitSyn
--                          state. If buffering the file fails or if the engine is already sending
--                          nothing will happen and false is returned.
--------------------------------------------------------------------------------------------------*/
bool kgp::IoEngine::StartFileSend(const std::string& filename, const std::string& address, const short& port)
{
    QMutexLocker locker(&mMutex);

    // If not already sending
    if (!mState.dataSent)
    {
        DependencyManager::Instance().Logger().Log("Sending file " + filename + " to " + address);
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
        DependencyManager::Instance().Logger().Error("Already sending");
        return false;
    }
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::send
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void kgp::IoEngine::send(const Packet& packet, const QHostAddress& address, const short& port)
--                              packet: The packet to send.
--                              address: The address to send the packet to.
--                              port: The port to send the packet on.
--
-- NOTES:
--                          Sends packet to address on port port using UDP and logs the sent packet.
--------------------------------------------------------------------------------------------------*/
void kgp::IoEngine::send(const Packet& packet, const QHostAddress& address, const short& port)
{
    mSocket.writeDatagram((char *)&packet, sizeof(packet), address, port);
    DependencyManager::Instance().Logger().Log("Sending packet ...");
    DependencyManager::Instance().Logger().LogPacket(packet, address);
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::sendFrames
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void kgp::IoEngine::sendFrames(std::vector<SlidingWindow::Frame> list, const QHostAddress& client, const short& port)
--                              list: The list of frames to send.
--                              client: The host to send to.
--                              port: The port to send on.
--
-- NOTES:
--                          Sends the list of frames to client on port port.
--------------------------------------------------------------------------------------------------*/
void kgp::IoEngine::sendFrames(std::vector<SlidingWindow::Frame> list, const QHostAddress& client, const short& port)
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

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::sendWindow
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void kgp::IoEngine::sendWindow(const QHostAddress& client, const short& port)
--                              client: The client to send to.
--                              port: The port to send on.
--
-- NOTES:
--                          Sends the window to the client on port port. Will grab a list of frames
--                          from the sliding window and then sends it to the client. If the window
--                          has no more frames to send then an EOT packet is sent instead.
--------------------------------------------------------------------------------------------------*/
void kgp::IoEngine::sendWindow(const QHostAddress& client, const short& port)
{
    if (mWindow.IsEot())
    {
        DependencyManager::Instance().Logger().Log("Transmission finished, sending EOT");
        sendEot(client, port);
        Reset();
    }
    else
    {
        DependencyManager::Instance().Logger().Log("Transmission unfinished, sending data");
        std::vector<SlidingWindow::Frame> frames;
        mWindow.GetNextFrames(frames);
        sendFrames(frames, client, port);
        restartRcvTimer();
        mState.dataSent = true;
    }
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::newDataHandler
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void kgp::IoEngine::newDataHandler()
--
-- NOTES:
--                          Callback function for when new data appears on the socket to be read.
--                          Will read packets from the socket until all packets are handled. After
--                          validating the packet size, the packet is handled according to protocol.
--------------------------------------------------------------------------------------------------*/
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
            DependencyManager::Instance().Logger().Error("Not enough data was read from " + datagram.senderAddress().toString().toStdString());
            continue;
        }

        memcpy(&buffer, datagram.data().data(), datagram.data().size());

        // Restart idle timeout
        restartIdleTimer();

        // Log receive packet here
        DependencyManager::Instance().Logger().Log("Receiving packet ...");
        DependencyManager::Instance().Logger().LogPacket(buffer, datagram.senderAddress());

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
                DependencyManager::Instance().Logger().Error("SYN received while in invalid state from " + datagram.senderAddress().toString().toStdString());
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
                        DependencyManager::Instance().Logger().Error("ACK received for SYN while in invalid state from " + datagram.senderAddress().toString().toStdString());
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
                            DependencyManager::Instance().Logger().Error("Unexpected ACK received(" + QString::number(buffer.Header.AckNumber).toStdString() + ")");
                        }

                    }
                }
            }
            else
            {
                DependencyManager::Instance().Logger().LogInvalidSender(mClientAddress, mClientPort, datagram.senderAddress(), datagram.senderPort());
            }
            break;
        case PacketType::DATA:
            if (mState.wait)
            {
                // Check if it is the incoming packet is a previous packet or next packet
                if (buffer.Header.SequenceNumber <= mState.seqNum)
                {
                    DependencyManager::Instance().Logger().Log("Valid packet received");

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
                    DependencyManager::Instance().Logger().Error("Invalid packet received, expecting sequence number " + QString::number(mState.seqNum).toStdString());
                }
            }
            else
            {
                DependencyManager::Instance().Logger().Error("Data received while in invalid state from " + datagram.senderAddress().toString().toStdString());
            }
            break;
        case PacketType::EOT:
            if (mState.wait)
            {
                // Valid EOT was received so reset
                DependencyManager::Instance().Logger().Log("EOT received, resetting state");
                Reset();
            }
            else
            {
                DependencyManager::Instance().Logger().Error("EOT received while in invalid state from " + datagram.senderAddress().toString().toStdString());
            }
            break;
        }
    }
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::IoEngine::run
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void kgp::IoEngine::run()
--
-- NOTES:
--                          Overloaded run function of QThread::run. This is the main function of the
--                          thread. The thread is started and this function is run whenever there is
--                          an active connection and is stopped when the connection is closed. This
--                          function is responsible for checking the timeouts and handling them
--                          accordingly.
--------------------------------------------------------------------------------------------------*/
void kgp::IoEngine::run()
{
    // Only run when there is a connection and not idle
    while (!mState.idle)
    {
        checkTimers();

        // If idle timeout has been reached
        if (mState.timeoutIdle)
        {
            DependencyManager::Instance().Logger().Log("Idle timeout reached");
            Reset();
        }

        // If receive timeout has been reached
        if (mState.timeoutRcv)
        {
            DependencyManager::Instance().Logger().Log("Receive timeout reached");

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
                DependencyManager::Instance().Logger().Log("Resending pending packets");
                std::vector<SlidingWindow::Frame> pendingFrames;
                mWindow.GetPendingFrames(pendingFrames);
                sendFrames(pendingFrames, mClientAddress, mClientPort);
                restartRcvTimer();
                restartIdleTimer();
            }
            // If ACKs timed out
            else if (mState.wait)
            {
                DependencyManager::Instance().Logger().Log("Resending ACK");
                // Resend all ACKs
                ackPacket(mState.seqNum, mClientAddress, mClientPort);
                restartRcvTimer();
                restartIdleTimer();
            }
            else
            {
                // This should never happen
                DependencyManager::Instance().Logger().Error("Receive timeout reached while in invalid state.");
            }
        }
    }
}