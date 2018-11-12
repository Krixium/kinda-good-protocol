#include "KindaGoodProtocol.h"
#include "DependancyManager.h"

KindaGoodProtocol::KindaGoodProtocol(QWidget *parent)
	: QMainWindow(parent)
	, mIo(true, this)
{
	ui.setupUi(this);
	kgp::DependancyManager::Instance().GetLogger().Log("Main window initialized");
	mIo.Start();
}

KindaGoodProtocol::~KindaGoodProtocol()
{
	kgp::DependancyManager::Instance().GetLogger().Log("Program exiting");
	mIo.Stop();
}
