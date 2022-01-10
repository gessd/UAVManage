#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include "libhvsetting.h"


class ResendMessage : public QThread
{
	Q_OBJECT

public:
	enum {
		MessageUnknown = 0,
		MessageTimeout = -404
	};

	ResendMessage(hv::TcpClient* tcpClient, unsigned int againNum, unsigned int timeout, 
		QByteArray arrData, QByteArray arrAgainData, int messageid, QObject *parent=nullptr);
	~ResendMessage();
	void stopThread();
	int getResult();
private slots:
	void onResult(int res, int id);
protected:
	void run();
private:
	hv::TcpClient* m_pTcpClient;
	unsigned int m_unAgainNumber;
	unsigned int m_unTimeout;
	QByteArray m_arrData;
	QByteArray m_arrAgainData;
	QMutex m_mutexStop;
	bool m_bStop;
	QMutex m_mutexResult;
	int m_nResult;
	int m_nMessageID;
};
