#pragma once

#include <memory>
#include <string>

#include <QDateTime>
#include <QDebug>
#include <QFile>

#include "res.h"

namespace kgp
{
	class Logger
	{
	public:
		static std::unique_ptr<QFile> LogFile;

	public:
		Logger() = delete;
		~Logger() = delete;

		inline static void Initialize()
		{
			LogFile->close();
			LogFile = std::make_unique<QFile>(LOG_FILE);
			LogFile->open(QIODevice::Append | QIODevice::Text);
		}

		inline static void Cleanup()
		{
			LogFile->close();
		}

		inline static void Log(const std::string& msg)
		{
			QString line("[ " + QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss") + " KGP ]: " + msg.c_str() + '\n');

			qDebug() << line;
			LogFile->write(line.toStdString().c_str());
		}
	};
}
