#pragma once

#include <string>

#include <QHostAddress>
#include <QMutex>
#include <QMutexLocker>
#include <QUdpSocket>
#include <QThread>
#include <QTime>

#include "DependencyManager.h"
#include "res.h"
#include "SlidingWindow.h"

namespace kgp
{
    class IoEngine : public QThread
    {
        Q_OBJECT

    private:
        QMutex mMutex;

        struct State mState;
        
        QUdpSocket mSocket;
        QHostAddress mClientAddress;
        short mClientPort;

        QTime mRcvTimer;
        QTime mIdleTimer;
        SlidingWindow mWindow;

    protected:
        void run();

    public:
        IoEngine(QObject *parent = nullptr);
        virtual ~IoEngine();

        void Start();
        void Stop();

        void Reset();

        bool StartFileSend(const std::string& filename, const std::string& address, const short& port);


        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::IoEngine::SetReceiveWindowSize
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::IoEngine::SetReceiveWindowSize(cosnt quint64 size)
        --                              size: The new size of the receiving window.
        --
        -- NOTES:
        --                          Setter for the receiving window size.
        --------------------------------------------------------------------------------------------------*/
        inline void SetReceiveWindowSize(const quint64 size) { mState.rcvWindowSize = size; }

    private:
        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::IoEngine::restartRcvTimer
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::IoEngine::restartRcvTimer()
        --
        -- NOTES:
        --                          Restarts the receive timer.
        --------------------------------------------------------------------------------------------------*/
        inline void restartRcvTimer()
        {
            mRcvTimer.start();
            mState.timeoutRcv = false;
        }

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::IoEngine::restartIdleTimer
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::IoEngine::restartIdleTimer()
        --
        -- NOTES:
        --                          Restarts the idle timer.
        --------------------------------------------------------------------------------------------------*/
        inline void restartIdleTimer()
        {
            mIdleTimer.start();
            mState.timeoutIdle = false;
        }

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::IoEngine::checkTimers
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::IoEngine::checkTimers()
        --
        -- NOTES:
        --                          Checks the timers and sets the state accordingly.
        --------------------------------------------------------------------------------------------------*/
        inline void checkTimers()
        {
            QMutexLocker locker(&mMutex);
            if (mRcvTimer.elapsed() > Timeout::RCV) mState.timeoutRcv = true;
            if (mIdleTimer.elapsed() > Timeout::IDLE) mState.timeoutIdle = true;
        }

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::IoEngine::createSynPacket
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void ckgp::IoEngine::reateSynPacket(Packet kgp::IoEngine::*buffer)
        --                              buffer: A pointer to the packet buffer to fill.
        --
        -- NOTES:
        --                          Creates a SYN packet and puts it into buffer.
        --------------------------------------------------------------------------------------------------*/
        inline void createSynPacket(Packet *buffer)
        {
            memset(buffer, 0, sizeof(buffer));
            buffer->Header.AckNumber = 0;
            buffer->Header.SequenceNumber = 0;
            buffer->Header.WindowSize = mState.rcvWindowSize;
            buffer->Header.PacketType = PacketType::SYN;
            buffer->Header.DataSize = 0;
        }

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::IoEngine::ackPacket
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::IoEngine::ackPacket(const quint64& seqNum, const QHostAddress& sender, const shortkgp::IoEngine::& port)
        --                              seqNum: The sequence number to ACK.
        --                              sender: The sender of the packet that is being ACK'd.
        --                              port: The port of the packet that is being ACK'd.
        --
        -- NOTES:
        --                          Creates an ACK packet for seqNum and sends it to sender on port port.
        --------------------------------------------------------------------------------------------------*/
        inline void ackPacket(const quint64& seqNum, const QHostAddress& sender, const short& port)
        {
            Packet res;
            memset(&res, 0, sizeof(res));
            res.Header.AckNumber = seqNum;
            res.Header.SequenceNumber = 0;
            res.Header.WindowSize = mState.rcvWindowSize;
            res.Header.PacketType = PacketType::ACK;
            res.Header.DataSize = 0;

            send(res, sender, port);
        }

        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::IoEngine::sendEot
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::IoEngine::sendEot(const QHostAddress& receiver, const shortkgp::IoEngine::& port)
        --                              receiver: The host to send the EOT to.
        --                              port: The port to send the EOT on.
        --
        -- NOTES:
        --                          Sends an EOT packet to receiver on port port.
        --------------------------------------------------------------------------------------------------*/
        inline void sendEot(const QHostAddress& receiver, const short& port)
        {
            Packet res;
            memset(&res, 0, sizeof(res));
            res.Header.PacketType = PacketType::EOT;
            res.Header.SequenceNumber = 0;
            res.Header.AckNumber = 0;
            res.Header.WindowSize = 0;
            res.Header.DataSize = 0;
            send(res, receiver, port);
        }

        void send(const Packet& packet, const QHostAddress& address, const short& port);
        void sendFrames(std::vector<SlidingWindow::Frame> list, const QHostAddress& client, const short& port);
        void sendWindow(const QHostAddress& client, const short& port);

    private slots:
        void newDataHandler();

    signals:
        void dataRead(const char *data, const size_t& size);

    };
}


