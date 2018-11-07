#pragma once

#include <QThread>
#include <QUdpSocket>

class Io : public QThread
{
protected:
	bool mIsRunning;
	QUdpSocket mSocket;

public:
	inline Io(const bool running = true, QObject *parent = nullptr)
		: QThread(parent)
		, mIsRunning(running)
		, mSocket(this)
	{
	}

	virtual ~Io() = default;

	void Start() { start(); }
	void Stop() { mIsRunning = false; }

protected:
	void run() = 0;
};

