#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include "definesetting.h"

class ResendMessage : public QThread
{
	Q_OBJECT

public:
	ResendMessage(QString comName, QString qstrName, unsigned int againNum, unsigned int timeout,
		QByteArray arrData, QByteArray arrAgainData, int messageid, _DeviceStatus initStatus = DeviceWaiting
		, bool bauto = false, QObject *parent=nullptr);
	~ResendMessage();
	QString getCommandName();
	int getResult();
	void setAutoDelete(bool bauto);
private slots:
	void stopThread(); 
private slots:
	void onResult(QString name, int res, int id);
protected:
	void run();
signals:
	void sigSendMessage(bool again, QByteArray data);
private:
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
	QString m_qstrName;
	QString m_qstrComName;
};
