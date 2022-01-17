#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include "libhvsetting.h"

enum _DeviceStatus
{
	DeviceMessageToimeout = -404,
	DeviceDataError = -3,
	DeviceUnConnect = -2,
	DeviceWaiting = -1,
	DeviceDataSucceed = 0
};

class ResendMessage : public QThread
{
	Q_OBJECT

public:
	ResendMessage(hv::TcpClient* tcpClient, unsigned int againNum, unsigned int timeout, 
		QByteArray arrData, QByteArray arrAgainData, int messageid, _DeviceStatus initStatus = DeviceWaiting
		, bool bauto = false, QObject *parent=nullptr);
	~ResendMessage();
	void stopThread();
	int getResult();
	void setAutoDelete(bool bauto);
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
	bool m_bAutoDelete;
	_DeviceStatus m_initStatus;
};
