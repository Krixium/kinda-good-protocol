#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_KindaGoodProtocol.h"

#include "Logger.h"
#include "Transmitter.h"
#include "Receiver.h"

class KindaGoodProtocol : public QMainWindow
{
	Q_OBJECT

private:
	kgp::Transmitter mTx;
	kgp::Receiver mRx;

public:
	KindaGoodProtocol(QWidget *parent = Q_NULLPTR);
	~KindaGoodProtocol();

private:
	Ui::KindaGoodProtocolClass ui;

};
