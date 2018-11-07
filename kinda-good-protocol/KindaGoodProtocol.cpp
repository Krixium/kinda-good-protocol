#include "KindaGoodProtocol.h"

#include "res.h"

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
	
	kgp::Log("Main window initialized");
}
