#include "resendmessage.h"
#include <QTime>
#include <QDebug>

ResendMessage::ResendMessage(QString comName, QString qstrName, unsigned int againNum, unsigned int timeout,
	QByteArray arrData, QByteArray arrAgainData, int messageid, _DeviceStatus initStatus, bool bauto, QObject *parent)
	: QThread(parent)
{
	m_qstrName = qstrName;
	m_unAgainNumber = againNum;
	m_unTimeout = timeout;
	m_arrData = arrData;
	m_arrAgainData = arrAgainData;
	m_nResult = initStatus;
	m_initStatus = initStatus;
	m_bStop = false;
	m_nMessageID = messageid;
	m_qstrComName = comName;
	setAutoDelete(bauto);
	//qDebug() << "----新建线程" << qstrName << this;
	connect(this, &QThread::finished, [this]() {
		//qDebug() << "----线程执行完成" << this;
		if (false == m_bAutoDelete) return;
		deleteLater();
		});
}

ResendMessage::~ResendMessage()
{
	if (isRunning()) {
		stopThread();
	}
	//qDebug() << "----线程释放" << this;
}

QString ResendMessage::getCommandName()
{
	return m_qstrComName;
}

void ResendMessage::stopThread()
{
	//qDebug() << "----停止线程" << m_qstrName << this;
	m_mutexStop.lock();
	m_bStop = true;
	m_mutexStop.unlock();
	//等待线程停止
	while (isRunning());
}

int ResendMessage::getResult()
{
	QMutexLocker locker(&m_mutexResult);
	return m_nResult;
}

void ResendMessage::setAutoDelete(bool bauto)
{
	m_bAutoDelete = bauto;
}

void ResendMessage::onResult(QString name, int res, int id)
{
	if (m_nMessageID != id) return;
	//qDebug() << "----收到结果" << m_qstrName << res << id << this;
	m_mutexResult.lock();
	m_nResult = res;
	m_mutexResult.unlock();
	stopThread();
}

void ResendMessage::run()
{
	//因第一次发送与重发内容会不同所以先发第一遍
	QTime time;
	time.start();
	unsigned int index = 0;
	emit sigSendMessage(m_arrData);
	while (!m_bStop) {
		//超时重发
		msleep(10);
		if (m_unTimeout > time.elapsed()) continue;	
		index++;
		if (index > m_unAgainNumber) {
			//超出重发次数
			m_bStop = true;
			m_mutexResult.lock();
			if (m_initStatus == m_nResult) m_nResult = DeviceMessageToimeout;
			m_mutexResult.unlock();
			return;
		}
		emit sigSendMessage(m_arrAgainData);
		time.restart();
	}
}
