#include "KindaGoodProtocol.h"
#include "DependancyManager.h"

#include <QPushButton>

KindaGoodProtocol::KindaGoodProtocol(QWidget *parent)
	: QMainWindow(parent)
	, mIo(true, this)
{
	ui.setupUi(this);

	// TESTING ONLY
	mFile = new QFile("dump.txt");
	mFile->open(QIODevice::WriteOnly | QIODevice::Text);
	Q_ASSERT(mFile->isOpen() && mFile->isWritable());
	// END

	kgp::DependancyManager::Instance().Logger().Log("Main window initialized");

	connect(&mIo, &kgp::IoEngine::dataRead, this, &KindaGoodProtocol::writeBytesToFile);
	connect(ui.buttonSend, &QPushButton::pressed, this, &KindaGoodProtocol::startSend);
}

KindaGoodProtocol::~KindaGoodProtocol()
{
	kgp::DependancyManager::Instance().Logger().Log("Program exiting");
	mIo.Stop();

	// TEST ONLY
	mFile->close();
	delete mFile;
	// END
}

void KindaGoodProtocol::startSend()
{
	mIo.StartFileSend("alice.txt", "192.168.0.18", kgp::PORT);
}

void KindaGoodProtocol::writeBytesToFile(const char *data, const size_t& size)
{
	mFile->write(data, size);
	mFile->flush();
}
