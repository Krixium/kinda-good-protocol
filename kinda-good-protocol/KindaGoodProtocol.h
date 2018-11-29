#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_KindaGoodProtocol.h"

#include "Logger.h"
#include "IoEngine.h"

class KindaGoodProtocol : public QMainWindow
{
    Q_OBJECT

private:
    kgp::IoEngine mIo;
    
    QFile *mOutputFile;

    QString mFileName;

public:
    KindaGoodProtocol(QWidget *parent = Q_NULLPTR);
    ~KindaGoodProtocol();

private:
    Ui::KindaGoodProtocolClass ui;

private slots:
    void startSend();

    void writeBytesToFile(const char *data, const size_t& size);

    void selectFileToSend();
};
