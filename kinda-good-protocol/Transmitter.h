#pragma once

#include "Io.h"

namespace kgp
{
	class Transmitter : public Io
	{
	public:
		Transmitter(const bool running = true, QObject *parent = nullptr);
		~Transmitter() = default;

	protected:
		void run();

	};
}
