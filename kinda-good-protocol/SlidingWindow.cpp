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
-- NOTES:
--							Reads the entire contents of the file and buffers it in memory. The
--							given file will be opened by this function if a closed file is passed,
--							and the file will always be closed after it is read.
--------------------------------------------------------------------------------------------------*/
void kgp::SlidingWindow::BufferFile(QFile& file)
{
	// Reset state of window
	Reset();

	// Open the file if needed
	if (!file.isOpen()) file.open(QIODevice::ReadOnly);

	// Read the entire file into the buffer
	mBuffer.append(file.readAll());

	// Close the file
	file.close();
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
-- INTERFACE:				quint64 GetNextFrame(char *data)  
--								data: The buffer where data will be read into.
--
-- RETURN:					The ACK number required to ACK the data read into data. This is also the
--							current position of the pointer.
--
-- NOTES:
--							Zero outs data, then will as much data is available. If the remaining
--							window space is less than the size of the data portion in a packet, it
--							will read the rest of the window, otherwise it will read enough data to
--							fill the packet. After reading the pointer is advanced to the next unread
--							byte and is returned.
--------------------------------------------------------------------------------------------------*/
quint64 kgp::SlidingWindow::GetNextFrame(char *data)
{
	// Clear buffer just in case
	memset(data, 0, Size::DATA);

	// If there is enough space in the window ...
	if (mPointer + Size::DATA <= mHead + mSize)
	{
		// Read data and increment the pointer
		memcpy(data, mBuffer.data() + mPointer, Size::DATA);
		mPointer += Size::DATA;
	}
	// If there is data remaining but less than Size::Data
	else if (mHead + mSize - mPointer > 0)
	{
		const quint64 remaining = mHead + mSize - mPointer;
		memcpy(data, mBuffer.data() + remaining, remaining);
		mPointer += remaining;
	}

	// return the window pointer
	return mPointer;
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
void kgp::SlidingWindow::AckFrame(const quint64& ackNum)
{
	// If sequence number of the acked frame is between the window head and the window pointer
	if (ackNum > mHead && ackNum <= mPointer)
	{
		// Shift the window pointer
		mHead = ackNum;
		DependancyManager::Instance().GetLogger().Log("Advancing window head to " + QString::number(ackNum).toStdString());
	}
	else
	{
		DependancyManager::Instance().GetLogger().Log("Invalid sequence number " + QString::number(ackNum).toStdString() + " received");
	}
}
