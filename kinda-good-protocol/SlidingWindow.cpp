#include "DependancyManager.h"
#include "SlidingWindow.h"

kgp::SlidingWindow::SlidingWindow(const quint64& size)
	: mWindowSize(size)
{
	Reset();
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                BufferFile
--
-- DATE:                    November 8, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:				void BufferFile(QFile& file)  
--								file: The file to buffer.
--
-- RETURN:
--							False if the file could not be read, true otherwise.
--
-- NOTES:
--							Reads the entire contents of the file and buffers it in memory. The
--							given file will be opened by this function if a closed file is passed,
--							and the file will always be closed after it is read.
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
			DependancyManager::Instance().Logger().Error("Could not buffer the file: " + file.fileName().toStdString());
			return false;
		}
	}

	// Read the entire file into the buffer
	mBuffer.append(file.readAll());
	DependancyManager::Instance().Logger().Log(QString::number(mBuffer.size()).toStdString() + " bytes were buffered");

	// Close the file
	file.close();

	return true;
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                GetNextFrame
--
-- DATE:                    November 8, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:				  
--
-- RETURN:					
--
-- NOTES:
--------------------------------------------------------------------------------------------------*/
void kgp::SlidingWindow::GetNextFrames(std::vector<FrameWrapper>& list)
{
	FrameWrapper frameWrapper;

	// While the pointer is less than the window size and less than buffer size
	while (mPointer < mHead + mWindowSize && mPointer < mBuffer.size())
	{
		memset(&frameWrapper, 0, sizeof(frameWrapper));
		frameWrapper.seqNum = mPointer;
		frameWrapper.data = mBuffer.data() + mPointer;

		// If the window does not have enough space for a whole packet
		if (mPointer + Size::DATA > mHead + mWindowSize)
		{
			// Set the size to the remaining window size
			frameWrapper.size = (mHead + mWindowSize) - mPointer;

			// Check for buffer overflow
			if (mPointer + frameWrapper.size > mBuffer.size())
			{
				// Set the size
				frameWrapper.size = mBuffer.size() - mPointer;
				// Remember that last packet has been sent
				mLastPacketState.pending = true;
				mLastPacketState.seqNum = frameWrapper.seqNum;
			}
		}
		// Normal case where the window has enough space for a whole packet
		else
		{
			// Check for buffer overflow
			if (mPointer + Size::DATA > mBuffer.size())
			{
				// Set the size
				frameWrapper.size = mBuffer.size() - mPointer;
				// Remember that last packet has been sent
				mLastPacketState.pending = true;
				mLastPacketState.seqNum = frameWrapper.seqNum;			
			}
			else
			{
				frameWrapper.size = Size::DATA;
			}
		}

		// Increment pointer and save the frame to the list
		mPointer += frameWrapper.size;
		list.push_back(frameWrapper);
	}
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                GetPendingFrames
--
-- DATE:                    November 8, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:				  
--
-- RETURN:					
--
-- NOTES:
--------------------------------------------------------------------------------------------------*/
void kgp::SlidingWindow::GetPendingFrames(std::vector<FrameWrapper>& list)
{
	quint64 tmpPointer = mHead;

	while (tmpPointer <= mPointer)
	{
		FrameWrapper frameWrapper;
		frameWrapper.seqNum = tmpPointer;
		frameWrapper.data = mBuffer.data() + tmpPointer;

		// If reading default size of data would exceed window size
		if (tmpPointer + Size::DATA > mHead + mWindowSize)
		{
			// Size is distance between the two pointers
			frameWrapper.size = mHead + mWindowSize - tmpPointer;
		}
		else
		{
			frameWrapper.size = Size::DATA;
		}

		// Increment pointer
		tmpPointer += frameWrapper.size;
		list.push_back(frameWrapper);
	}
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                AckFrame
--
-- DATE:                    November 8, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:				void AckFrame(const quint64& ackNum)  
--								ackNum: The ACK number where 0 is the start of the buffered data.
--
-- NOTES:
--							Will advance the head to the ACK number if it is valid. The pointer is 
--							unchanged.
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

	// If sequence number of the acked frame is between the window head and the window pointer
	if (ackNum > mHead && ackNum <= mPointer)
	{
		// Shift the window pointer
		mHead = ackNum;
		DependancyManager::Instance().Logger().Log("Advancing window head to " + QString::number(ackNum).toStdString());
		return true;
	}
	else
	{
		DependancyManager::Instance().Logger().Log("Invalid sequence number " + QString::number(ackNum).toStdString() + " received");
		return false;
	}
}
