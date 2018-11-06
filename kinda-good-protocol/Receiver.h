#pragma once

#include <QThread>

namespace kgp
{
	class Receiver : public QThread
	{
	public:
		Receiver(QObject *parent = nullptr);
		~Receiver() = default;

	protected:
		void run();

	};
}

