#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_KindaGoodProtocol.h"

class KindaGoodProtocol : public QMainWindow
{
	Q_OBJECT

public:
	KindaGoodProtocol(QWidget *parent = Q_NULLPTR);

private:
	Ui::KindaGoodProtocolClass ui;
};
