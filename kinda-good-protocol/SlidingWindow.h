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
		struct FrameWrapper
		{
			quint64 seqNum;
			size_t size;
			char *data;
		};

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

		inline bool IsEOT()
		{
			return mBuffer.size() == mHead;
		}

		void BufferFile(QFile& file);

		void GetNextFrames(std::vector<FrameWrapper>& list);
		void GetPendingFrames(std::vector<FrameWrapper>& list);
		bool AckFrame(const quint64& ackNum);
	};
}
