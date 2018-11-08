#pragma once

#include <QByteArray>
#include <QFile>

#include "res.h"

namespace kgp
{
	class SlidingWindow
	{
	private:
		quint64 mHead;
		quint64 mSize;
		quint64 mPointer;

		QByteArray mBuffer;

	public:
		SlidingWindow(const quint64& size = Size::WINDOW);
		~SlidingWindow() = default;

		inline void Reset()
		{
			mHead = mPointer = 0;
			mBuffer.clear();
		}

		void BufferFile(QFile& file);

		quint64 GetNextFrame(char *data);
		void AckFrame(const Packet& packet);
	};
}
