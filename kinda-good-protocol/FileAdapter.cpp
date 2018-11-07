#include "DependancyManager.h"
#include "FileAdapter.h"

kgp::FileAdapter::FileAdapter(const std::string& filename, const QIODevice::OpenMode& mode, const quint64& windowSize)
	: mWindowHead(0)
	, mWindowPointer(0)
	, mWindowSize(windowSize)
{
	mFile = std::make_unique<QFile>(filename);
	mFile->open(mode);
}

kgp::FileAdapter::~FileAdapter()
{
	mFile->close();
}

quint64 kgp::FileAdapter::GetNextFrame(char *data)
{
	// If there is enough space in the window ...
	if (mWindowPointer + Size::DATA <= mWindowHead + mWindowSize)
	{
		// Read data and increment the pointer
		mWindowPointer += mFile->read(data, Size::DATA);
	}

	// return the window pointer
	return mWindowPointer;
}

void kgp::FileAdapter::AckFrame(const Packet& packet)
{
	// If sequence number of the acked frame is between the window head and the window pointer
	if (packet.Header.AckNumber > mWindowHead && packet.Header.AckNumber <= mWindowPointer)
	{
		// Shift the window pointer
		mWindowHead = packet.Header.AckNumber;
		DependancyManager::Instance().GetLogger().Log("Shifting window head to " + QString::number(packet.Header.AckNumber).toStdString());
	}
	else
	{
		DependancyManager::Instance().GetLogger().Log("Invalid sequence number " + QString::number(packet.Header.AckNumber).toStdString() + " received");
	}
}
