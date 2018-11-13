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
		QFile mLogFile;

	public:
		inline Logger()
			: mLogFile(LOG_FILE)
		{
			if (mLogFile.isOpen()) mLogFile.close();
			mLogFile.open(QIODevice::Append | QIODevice::Text);
		}

		inline ~Logger()
		{
			if (mLogFile.isOpen()) mLogFile.close();
		}

		inline void Log(const std::string& msg)
		{
			QString line("[ " + QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss") + " Log ]: " + msg.c_str() + '\n');
			write(line);
		}

		inline void Error(const std::string& msg)
		{
			QString line("[ " + QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss") + " Error ]: " + msg.c_str() + '\n');
			write(line);
		}

	private:
		inline void write(QString line)
		{
			qDebug() << line;
			mLogFile.write(line.toStdString().c_str());
		}
	};
}
