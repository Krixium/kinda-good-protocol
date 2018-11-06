#pragma once

#include <QThread>

namespace kgp
{
	class Transmitter : public QThread
	{
	public:
		Transmitter(QObject *parent = nullptr);
		~Transmitter() = default;

	protected:
		void run();

	};
}
