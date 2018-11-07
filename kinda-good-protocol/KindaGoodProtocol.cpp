#include "KindaGoodProtocol.h"

KindaGoodProtocol::KindaGoodProtocol(QWidget *parent)
	: QMainWindow(parent)
	, mTx(true, this)
	, mRx(true, this)
{
	ui.setupUi(this);

	kgp::Logger::Initialize();

	mTx.Start();
	mRx.Start();

	mTx.Stop();
	mRx.Stop();
	
	kgp::Logger::Log("Main window initialized");
}
