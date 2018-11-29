#include "DependencyManager.h"
#include "SlidingWindow.h"

kgp::SlidingWindow::SlidingWindow(const quint64& size)
    : mWindowSize(size)
{
    Reset();
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::SlidingWindow::BufferFile
--
-- DATE:                    November 8, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void BufferFile(QFile& file)  
--                              file: The file to buffer.
--
-- RETURN:
--                          False if the file could not be read, true otherwise.
--
-- NOTES:
--                          Reads the entire contents of the file and buffers it in memory. The
--                          given file will be opened by this function if a closed file is passed,
--                          and the file will always be closed after it is read.
--------------------------------------------------------------------------------------------------*/
bool kgp::SlidingWindow::BufferFile(QFile& file)
{
    // Reset state of window
    Reset();

    // Open the file if needed
    if (!file.isOpen())
    {
        // Return false if the file could not be opened
        if (!file.open(QIODevice::ReadOnly))
        {
            DependencyManager::Instance().Logger().Error("Could not buffer the file: " + file.fileName().toStdString());
            return false;
        }
    }

    // Read the entire file into the buffer
    mBuffer.append(file.readAll());
    DependencyManager::Instance().Logger().Log(QString::number(mBuffer.size()).toStdString() + " bytes were buffered");

    // Close the file
    file.close();

    return true;
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::SlidingWindow::GetNextFrames
--
-- DATE:                    November 8, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void kgp::SlidingWindow::GetNextFrames(std::vector<FrameWrapper>& list)
--                              list: The list that the frames will be put into.
--
-- NOTES:
--                          Starts at the window head and will attempt to grab frames until an entire
--                          window's worth has been grabbed. Will stop if the end of the buffer has
--                          been reached. The window pointer will be at the start of the the last
--                          frame that has been read this way.
--------------------------------------------------------------------------------------------------*/

void kgp::SlidingWindow::GetNextFrames(std::vector<Frame>& list)
{
    Frame frame;

    // While the pointer is less than the window size and less than buffer size
    while (mPointer < mHead + mWindowSize && mPointer < mBuffer.size())
    {
        memset(&frame, 0, sizeof(frame));
        frame.seqNum = mPointer;
        frame.data = mBuffer.data() + mPointer;

        // If the window does not have enough space for a whole packet
        if (mPointer + Size::DATA > mHead + mWindowSize)
        {
            // Set the size to the remaining window size
            frame.size = (mHead + mWindowSize) - mPointer;

            // Check for buffer overflow
            if (mPointer + frame.size > mBuffer.size())
            {
                // Set the size
                frame.size = mBuffer.size() - mPointer;
                // Remember that last packet has been sent
                mLastPacketState.pending = true;
                mLastPacketState.seqNum = frame.seqNum;
            }
        }
        // Normal case where the window has enough space for a whole packet
        else
        {
            // Check for buffer overflow
            if (mPointer + Size::DATA > mBuffer.size())
            {
                // Set the size
                frame.size = mBuffer.size() - mPointer;
                // Remember that last packet has been sent
                mLastPacketState.pending = true;
                mLastPacketState.seqNum = frame.seqNum;            
            }
            else
            {
                frame.size = Size::DATA;
            }
        }

        // Increment pointer and save the frame to the list
        mPointer += frame.size;
        list.push_back(frame);
    }
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::SlidingWindow::GetPendingFrames
--
-- DATE:                    November 8, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void kgp::SlidingWindow::GetPendingFrames(std::vector<FrameWrapper>& list)
--                              list: The list that the frames will be put into.
--
-- NOTES:
--                          Grabs all the pending frames, that is frames between the window head
--                          and the window pointer, and appends them to the list that was passed.
--------------------------------------------------------------------------------------------------*/
void kgp::SlidingWindow::GetPendingFrames(std::vector<Frame>& list)
{
    quint64 tmpPointer = mHead;

    while (tmpPointer <= mPointer)
    {
        Frame frame;
        frame.seqNum = tmpPointer;
        frame.data = mBuffer.data() + tmpPointer;

        // If reading default size of data would exceed window size
        if (tmpPointer + Size::DATA > mHead + mWindowSize)
        {
            // Size is distance between the two pointers
            frame.size = mHead + mWindowSize - tmpPointer;
        }
        else
        {
            frame.size = Size::DATA;
        }

        // Increment pointer
        tmpPointer += frame.size;
        list.push_back(frame);

        if (tmpPointer == mPointer) break;
    }
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                kgp::SlidingWindow::AckFrame
--
-- DATE:                    November 8, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void AckFrame(const quint64& ackNum)  
--                              ackNum: The ACK number where 0 is the start of the buffered data.
--
-- NOTES:
--                          Will advance the head to the ACK number if it is valid. The pointer is 
--                          unchanged. If the number that is ACK'd is the last ACK number for the
--                          session the EOT but is set. The window head will be set to the ACK
--                          number if the ACK number is somewhere in the range of
--                              window head < ACK number <= window pointer.
--------------------------------------------------------------------------------------------------*/
bool kgp::SlidingWindow::AckFrame(const quint64& ackNum)
{
    // Check if last packet has been sent or not
    if (mLastPacketState.pending)
    {
        // If the last packet was acked
        if (ackNum == mLastPacketState.seqNum)
        {
            // Set the state to show that last packet has been sent and acked
            mLastPacketState.pending = false;
            mLastPacketState.acked = true;
        }
    }

    // ACK could be for any packet that was sent previously
    if (ackNum <= mPointer)
    {
        // If ACK was for a packet in our window, shift the window
        if (ackNum > mHead)
        {
            mHead = ackNum;
        }
        DependencyManager::Instance().Logger().Log("Advancing window head to " + QString::number(ackNum).toStdString());
        return true;
    }
    else
    {
        DependencyManager::Instance().Logger().Log("Invalid sequence number " + QString::number(ackNum).toStdString() + " received");
        return false;
    }
}
