#include "res.h"

#include "Receiver.h"

kgp::Receiver::Receiver(const bool running, QObject *parent)
	: Io(running, parent)
{
	Log("Receiver thread initialized");
}

void kgp::Receiver::run()
{
	while (mIsRunning)
	{

	}

	quit();
	Log("Receiver thread finished");
}
