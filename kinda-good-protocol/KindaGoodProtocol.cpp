#include "KindaGoodProtocol.h"
#include "DependancyManager.h"

#include <QPushButton>

KindaGoodProtocol::KindaGoodProtocol(QWidget *parent)
	: QMainWindow(parent)
	, mIo(true, this)
{
	ui.setupUi(this);

	// TESTING ONLY
	file = new QFile("dump.txt");
	file->open(QIODevice::WriteOnly);
	// END

	kgp::DependancyManager::Instance().Logger().Log("Main window initialized");
	mIo.Start();

	connect(&mIo, &kgp::IoEngine::dataRead, this, &KindaGoodProtocol::writeBytesToFile);
	connect(ui.buttonSend, &QPushButton::pressed, this, &KindaGoodProtocol::startSend);
}

KindaGoodProtocol::~KindaGoodProtocol()
{
	kgp::DependancyManager::Instance().Logger().Log("Program exiting");
	mIo.Stop();

	// TEST ONLY
	file->close();
	delete file;
	// END
}

void KindaGoodProtocol::startSend()
{
	mIo.StartFileSend("alice.txt", "192.168.0.101", kgp::PORT);
}

void KindaGoodProtocol::writeBytesToFile(const char *data, const size_t& size)
{
	file->write(data, size);
}
