#pragma once

#include <memory>
#include <string>

#include <QFile>
#include <QFileSelector>

#include "res.h"

namespace kgp
{
	class FileAdapter
	{
	private:
		std::unique_ptr<QFile> mFile;

		quint64 mWindowHead;
		quint64 mWindowPointer;
		quint64 mWindowSize;

	public:
		FileAdapter(const std::string& filename, const QIODevice::OpenMode& mode, const quint64& windowSize = Size::WINDOW);
		~FileAdapter();

		quint64 GetNextFrame(char *data);
		void AckFrame(const Packet& packet);
	};
}
