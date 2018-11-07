#include "res.h"

#include "Logger.h"
#include "Receiver.h"

kgp::Receiver::Receiver(const bool running, QObject *parent)
	: Io(running, parent)
{
}

void kgp::Receiver::run()
{
	Logger::Log("Receiver thread started");

	while (mIsRunning)
	{

	}

	quit();
	Logger::Log("Receiver thread finished");
}
