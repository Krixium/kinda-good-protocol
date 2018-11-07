#pragma once

#include "Io.h"

namespace kgp
{
	class Receiver : public Io
	{
	private:
		kgp::State mCurrentState;
	public:
		Receiver(const bool running = true, QObject *parent = nullptr);
		~Receiver() = default;

	protected:
		void run();

	private slots:
		void newDataHandler();

	};
}

