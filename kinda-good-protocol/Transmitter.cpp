#include "res.h"

#include "Logger.h"
#include "Transmitter.h"

kgp::Transmitter::Transmitter(const bool running, QObject *parent)
	: Io(running, parent)
{
}

void kgp::Transmitter::run()
{
	Logger::Log("Transmitter thread started");

	while (mIsRunning)
	{

	}

	quit();
	Logger::Log("Transmitter thread finished");
}
