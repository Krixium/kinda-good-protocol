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

		struct LastPacketState
		{
			quint64 seqNum;
			bool pending;
			bool acked;
		};

	private:
		quint64 mHead;
		quint64 mWindowSize;
		quint64 mPointer;
		LastPacketState mLastPacketState;

		QByteArray mBuffer;

	public:
		SlidingWindow(const quint64& size = Size::WINDOW);
		~SlidingWindow() = default;

		inline void SetWindowSize(const quint64 size) { mWindowSize = size; }
		inline quint64 GetWindowSize() { return mWindowSize; }

		inline void Reset()
		{
			mHead = mPointer = 0;
			mBuffer.clear();
			memset(&mLastPacketState, 0, sizeof(mLastPacketState));
		}
		

		inline bool IsEOT()
		{
			return mLastPacketState.acked;
		}

		bool BufferFile(QFile& file);

		void GetNextFrames(std::vector<FrameWrapper>& list);
		void GetPendingFrames(std::vector<FrameWrapper>& list);
		bool AckFrame(const quint64& ackNum);
	};
}
