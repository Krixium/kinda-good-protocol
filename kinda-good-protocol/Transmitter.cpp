#include "res.h"

#include "DependancyManager.h"
#include "Transmitter.h"

kgp::Transmitter::Transmitter(const bool running, QObject *parent)
	: Io(running, parent)
{
	DependancyManager::Instance().GetLogger().Log("Transmitter thread initialized");
}

void kgp::Transmitter::run()
{
	DependancyManager::Instance().GetLogger().Log("Transmitter thread started");
	while (mIsRunning)
	{

	}

	DependancyManager::Instance().GetLogger().Log("Transmitter thread finished");
	quit();
}
