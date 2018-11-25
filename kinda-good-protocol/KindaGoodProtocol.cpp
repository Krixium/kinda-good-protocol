#include "KindaGoodProtocol.h"
#include "DependancyManager.h"

#include <QPushButton>

KindaGoodProtocol::KindaGoodProtocol(QWidget *parent)
	: QMainWindow(parent)
	, mIo(true, this)
{
	ui.setupUi(this);
	kgp::DependancyManager::Instance().Logger().Log("Main window initialized");
	mIo.Start();

	connect(&mIo, &kgp::IoEngine::dataRead, this, &KindaGoodProtocol::writeBytesToFile);
	connect(ui.buttonSend, &QPushButton::pressed, this, &KindaGoodProtocol::startSend);
}

KindaGoodProtocol::~KindaGoodProtocol()
{
	kgp::DependancyManager::Instance().Logger().Log("Program exiting");
	mIo.Stop();
}

void KindaGoodProtocol::startSend()
{
	mIo.StartFileSend("data.txt", "192.168.0.101", kgp::PORT);
}

void KindaGoodProtocol::writeBytesToFile(const char *data, const size_t& size)
{
	QFile file("dump.txt");
	file.open(QIODevice::WriteOnly);
	file.write(data, size);
	file.close();
}
