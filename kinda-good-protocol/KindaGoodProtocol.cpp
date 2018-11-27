#include "KindaGoodProtocol.h"
#include "DependancyManager.h"

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

KindaGoodProtocol::KindaGoodProtocol(QWidget *parent)
	: QMainWindow(parent)
	, mIo(true, this)
{
	ui.setupUi(this);

	// TESTING ONLY
	mFile = new QFile("dump.txt");
	mFile->open(QIODevice::WriteOnly);
	Q_ASSERT(mFile->isOpen() && mFile->isWritable());
	// END

	kgp::DependancyManager::Instance().Logger().Log("Main window initialized");

	connect(&mIo, &kgp::IoEngine::dataRead, this, &KindaGoodProtocol::writeBytesToFile);
	connect(ui.buttonSend, &QPushButton::pressed, this, &KindaGoodProtocol::startSend);
	connect(ui.selectFileButton, &QPushButton::pressed, this, &KindaGoodProtocol::selectFileToSend);
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
	if (mFileName.isEmpty())
	{
		mFileName = "alice.txt";
	}
	// Gets the IPv4 address from the UI and checks if it exists.
	std::string address = ui.addressLineEdit->text().toStdString();
	if (address.empty())
	{
		QMessageBox::warning(this, tr("Warning"), tr("No IP address specified!"));
		return;
	}
	mIo.StartFileSend(mFileName.toStdString(), address, kgp::PORT);
}

void KindaGoodProtocol::writeBytesToFile(const char *data, const size_t& size)
{
	mFile->write(data, size);
	mFile->flush();
}

void KindaGoodProtocol::selectFileToSend()
{
	mFileName = QFileDialog::getOpenFileName(this, tr("Select a file"));
	ui.selectedFileLineEdit->setText(mFileName);
}


