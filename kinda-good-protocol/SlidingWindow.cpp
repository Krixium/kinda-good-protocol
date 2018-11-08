#include "DependancyManager.h"
#include "SlidingWindow.h"

kgp::SlidingWindow::SlidingWindow(const quint64& size)
	: mSize(size)
{
	Reset();
}

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

void kgp::SlidingWindow::AckFrame(const Packet& packet)
{
	// If sequence number of the acked frame is between the window head and the window pointer
	if (packet.Header.AckNumber > mHead && packet.Header.AckNumber <= mHead)
	{
		// Shift the window pointer
		mHead = packet.Header.AckNumber;
		DependancyManager::Instance().GetLogger().Log("Shifting window head to " + QString::number(packet.Header.AckNumber).toStdString());
	}
	else
	{
		DependancyManager::Instance().GetLogger().Log("Invalid sequence number " + QString::number(packet.Header.AckNumber).toStdString() + " received");
	}
}
