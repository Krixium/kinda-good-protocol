/*---------------------------------------------------------------------------------------
-- SOURCE FILE:             KindaGoodProtocol.cpp
--
-- PROGRAM:                 KindaGoodProtocol
--
-- FUNCTIONS:               N/A
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNERS:               Benny Wang, William Murphy
--
-- PROGRAMMERS:             Benny Wang, William Murphy
--
-- NOTES:
--                          Main class of the application. The constructor for this class
--                          acts as the main entry point of the application and therefore
--                          serves as the parent for all other elements.
---------------------------------------------------------------------------------------*/
#include "KindaGoodProtocol.h"
#include "DependencyManager.h"

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileSystemWatcher>

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                KindaGoodProtocol::KindaGoodProtocol
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang, William Murphy
--
-- PROGRAMMER:              Benny Wang, William Murphy
--
-- INTERFACE:               KindaGoodProtocol::KindaGoodProtocol(QWidget *parent)
--                              parent: The parent QWidget.
--
-- NOTES:
--                          Constructor for KindaGoodProtocol. Also serves as the main entry point
--                          of the application. Initializes the IoEngine and wires up the UI.
--------------------------------------------------------------------------------------------------*/
KindaGoodProtocol::KindaGoodProtocol(QWidget *parent)
    : QMainWindow(parent)
    , mIo(this)
{
    ui.setupUi(this);

    mLogFileWatcher.addPath(kgp::LOG_FILE);

    mOutputFile = new QFile("output.txt");
    mOutputFile->open(QIODevice::WriteOnly);
    Q_ASSERT(mOutputFile->isOpen() && mOutputFile->isWritable());

    kgp::DependencyManager::Instance().Logger().Log("Main window initialized");

    // For streaming log text into the log browser
    mLogFile.setFileName(kgp::LOG_FILE);
    mLogFile.open(QIODevice::ReadOnly);
    mLogStream.setDevice(&mLogFile);

    ui.logBrowser->append(mLogStream.readAll());

    qDebug() << mLogFileWatcher.files();

    connect(&mLogFileWatcher, &QFileSystemWatcher::fileChanged, this, &KindaGoodProtocol::onLogFileUpdate);
    connect(&mIo, &kgp::IoEngine::dataRead, this, &KindaGoodProtocol::writeBytesToFile);
    connect(ui.buttonSend, &QPushButton::pressed, this, &KindaGoodProtocol::startSend);
    connect(ui.selectFileButton, &QPushButton::pressed, this, &KindaGoodProtocol::selectFileToSend);
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                KindaGoodProtocol::~KindaGoodProtocol
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang, William Murphy
--
-- PROGRAMMER:              Benny Wang, William Murphy
--
-- INTERFACE:               KindaGoodProtocol::~KindaGoodProtocol()
--
-- NOTES:
--                          Deconstructor for KindaGoodProtocol. Stops all IO and cleans up any
--                          resources that could be left open.
--------------------------------------------------------------------------------------------------*/
KindaGoodProtocol::~KindaGoodProtocol()
{
    kgp::DependencyManager::Instance().Logger().Log("Program exiting");
    mIo.Stop();

    mOutputFile->close();
    delete mOutputFile;
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                KindaGoodProtocol::startSend
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang, William Murphy
--
-- PROGRAMMER:              Benny Wang, William Murphy
--
-- INTERFACE:               void KindaGoodProtocol::startSend()
--
-- NOTES:
--                          A Qt slot, that when triggered, will make a call to the IO Engine to
--                          start sending.
--------------------------------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                KindaGoodProtocol::writeBytesToFile
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                Benny Wang
--
-- PROGRAMMER:              Benny Wang
--
-- INTERFACE:               void KindaGoodProtocol::writeBytesToFile(const char *data, const size_t& size)
--                              data: A pointer to the start of the data.
--                              size: The length of data to write.
--
-- NOTES:
--                          Writes characters to a file start at data for a length of size.
--------------------------------------------------------------------------------------------------*/
void KindaGoodProtocol::writeBytesToFile(const char *data, const size_t& size)
{
    mOutputFile->write(data, size);
    mOutputFile->flush();
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                KindaGoodProtocol::selectFileToSend
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                William Murphy
--
-- PROGRAMMER:              William Murphy
--
-- INTERFACE:               void KindaGoodProtocol::selectFileToSend()
--
-- NOTES:
--                          A Qt slot, that when triggered, will open a file selector dialog to get
--                          a file name from the user.
--------------------------------------------------------------------------------------------------*/
void KindaGoodProtocol::selectFileToSend()
{
    mFileName = QFileDialog::getOpenFileName(this, tr("Select a file"));
    ui.selectedFileLineEdit->setText(mFileName);
}

/*--------------------------------------------------------------------------------------------------
-- FUNCTION:                KindaGoodProtocol::onLogFileUpdate
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNER:                William Murphy
--
-- PROGRAMMER:              William Murphy
--
-- INTERFACE:               void KindaGoodProtocol::onLogFileUpdate()
--
-- NOTES:
--                          A Qt slot, that when triggered, updates the UI based on the contents
--                          of the log file.
--------------------------------------------------------------------------------------------------*/
void KindaGoodProtocol::onLogFileUpdate()
{
    qDebug() << "Log file changed";
    ui.logBrowser->append(mLogStream.readAll());
    // Add the file path again if it was removed
}