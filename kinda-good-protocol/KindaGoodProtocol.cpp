#include "KindaGoodProtocol.h"
#include "DependancyManager.h"

KindaGoodProtocol::KindaGoodProtocol(QWidget *parent)
	: QMainWindow(parent)
	, mTx(true, this)
	, mRx(true, this)
{
	ui.setupUi(this);

	mTx.Start();
	mRx.Start();

	mTx.Stop();
	mRx.Stop();
	
	kgp::DependancyManager::Instance().GetLogger().Log("Main window initialized");
}

KindaGoodProtocol::~KindaGoodProtocol()
{
	kgp::DependancyManager::Instance().GetLogger().Log("Program exiting");
}
