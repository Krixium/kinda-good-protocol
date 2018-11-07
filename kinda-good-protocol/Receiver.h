#pragma once

#include "Io.h"

#include <QThread>

namespace kgp
{
	class Receiver : public Io
	{
	public:
		Receiver(const bool running = true, QObject *parent = nullptr);
		~Receiver() = default;

	protected:
		void run();

	};
}

