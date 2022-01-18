#include "resendmessage.h"
#include <QTime>
#include <QDebug>

ResendMessage::ResendMessage(hv::TcpClient* tcpClient, unsigned int againNum, unsigned int timeout, 
	QByteArray arrData, QByteArray arrAgainData, int messageid, _DeviceStatus initStatus, bool bauto, QObject *parent)
	: QThread(parent)
{
	m_pTcpClient = tcpClient;
	m_unAgainNumber = againNum;
	m_unTimeout = timeout;
	m_arrData = arrData;
	m_arrAgainData = arrAgainData;
	m_nResult = initStatus;
	m_initStatus = initStatus;
	m_bStop = false;
	m_nMessageID = messageid;
	setAutoDelete(bauto);

	connect(this, &QThread::finished, [this]() {
		if (false == m_bAutoDelete) return;
		deleteLater();
		});
}

ResendMessage::~ResendMessage()
{
	if (isRunning()) {
		stopThread();
	}
}

void ResendMessage::stopThread()
{
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
	m_mutexResult.lock();
	m_nResult = res;
	m_mutexResult.unlock();
	stopThread();
}

void ResendMessage::run()
{
	if (!m_pTcpClient) return;
	//因第一次发送与重发内容会不同所以先发第一遍
	QTime time;
	time.start();
	unsigned int index = 0;
	m_pTcpClient->send(m_arrData.data(), m_arrData.length());
	while (!m_bStop) {
		//超时重发
		msleep(10);
		if(m_unTimeout > time.elapsed()) continue;
		index++;
		if (index > m_unAgainNumber) {
			//超出重发次数
			m_bStop = true;
			m_mutexResult.lock();
			if (m_initStatus == m_nResult) m_nResult = DeviceMessageToimeout;
			m_mutexResult.unlock();
			return;
		}
		m_pTcpClient->send(m_arrAgainData.data(), m_arrAgainData.length());
		time.restart();
	}
}
