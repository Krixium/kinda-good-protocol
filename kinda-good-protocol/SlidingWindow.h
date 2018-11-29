/*---------------------------------------------------------------------------------------
-- SOURCE FILE:             SlidingWindow.h
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
--                          A wrapper class for the sliding window.
---------------------------------------------------------------------------------------*/
#pragma once

#include <vector>

#include <QByteArray>
#include <QFile>

#include "res.h"

namespace kgp
{
    class SlidingWindow
    {
    public:
        struct Frame
        {
            quint64 seqNum;
            size_t size;
            char *data;
        };

        struct EotState
        {
            quint64 seqNum;
            bool pending;
            bool acked;
        };

    private:
        quint64 mHead;
        quint64 mWindowSize;
        quint64 mPointer;
        EotState mLastPacketState;

        QByteArray mBuffer;

    public:
        SlidingWindow(const quint64& size = Size::WINDOW);
        ~SlidingWindow() = default;

        inline void SetWindowSize(const quint64 size) { mWindowSize = size; }
        inline quint64 GetWindowSize() { return mWindowSize; }


        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::SlidingWindow::Reset
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               void kgp::SlidingWindow::Reset()
        --
        -- NOTES:
        --                          Sets all member variables of the sliding window to its default state.
        --------------------------------------------------------------------------------------------------*/
        inline void Reset()
        {
            mHead = mPointer = 0;
            mBuffer.clear();
            memset(&mLastPacketState, 0, sizeof(mLastPacketState));
        }
        
        /*--------------------------------------------------------------------------------------------------
        -- FUNCTION:                kgp::SlidingWindow::IsEot
        --
        -- DATE:                    November 27, 2018
        --
        -- REVISIONS:               N/A
        --
        -- DESIGNER:                Benny Wang
        --
        -- PROGRAMMER:              Benny Wang
        --
        -- INTERFACE:               bool kgp::SlidingWindow::IsEot()
        --
        -- NOTES:
        --                          Checks if all data has been read and ACK'd. Returns true if the last
        --                          frame of data has been ACK, false otherwise.
        --------------------------------------------------------------------------------------------------*/
        inline bool IsEot()
        {
            return mLastPacketState.acked;
        }

        bool BufferFile(QFile& file);

        void GetNextFrames(std::vector<Frame>& list);
        void GetPendingFrames(std::vector<Frame>& list);
        bool AckFrame(const quint64& ackNum);
    };
}
