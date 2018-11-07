#include "res.h"

#include "Transmitter.h"

kgp::Transmitter::Transmitter(const bool running, QObject *parent)
	: Io(running, parent)
{
	Log("Transmitter thread initialized");
}

void kgp::Transmitter::run()
{
	while (mIsRunning)
	{

	}

	quit();
	Log("Transmitter thread finished");
}
