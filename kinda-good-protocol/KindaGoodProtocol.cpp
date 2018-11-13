#include "KindaGoodProtocol.h"
#include "DependancyManager.h"

KindaGoodProtocol::KindaGoodProtocol(QWidget *parent)
	: QMainWindow(parent)
	, mIo(true, this)
{
	ui.setupUi(this);
	kgp::DependancyManager::Instance().Logger().Log("Main window initialized");
	mIo.Start();

	// Connect this to something to handle valid data
	//connect(&mIo, &IoEngine::dataRead, , );
}

KindaGoodProtocol::~KindaGoodProtocol()
{
	kgp::DependancyManager::Instance().Logger().Log("Program exiting");
	mIo.Stop();
}
