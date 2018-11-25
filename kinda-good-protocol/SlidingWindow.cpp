#include "DependancyManager.h"
#include "SlidingWindow.h"

kgp::SlidingWindow::SlidingWindow(const quint64& size)
	: mSize(size)
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
	// While there is still data that can be read
	while (mPointer + Size::DATA <= mHead + mSize)
	{
		FrameWrapper frameWrapper;
		frameWrapper.seqNum = mPointer;
		frameWrapper.data = mBuffer.data() + mPointer;

		// If remaining unread window data is less than default size
		if (mHead + mSize - mPointer < Size::DATA)
		{
			// Size is the distance between end of window and pointer
			frameWrapper.size = (mHead + mSize) - mPointer;
			Q_ASSERT(frameWrapper.size < Size::DATA);
		}
		else
		{
			frameWrapper.size = Size::DATA;
		}

		// Increment pointer
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

		// If remaining unread window data is less than default size
		if (mPointer - tmpPointer < Size::DATA)
		{
			// Size is distance between the two pointers
			frameWrapper.size = mPointer - tmpPointer;
			Q_ASSERT(frameWrapper.size < Size::DATA);
		}
		else
		{
			frameWrapper.size = Size::DATA;
		}

		// Increment pointer
		tmpPointer += frameWrapper.size;
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
