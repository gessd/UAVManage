#include "resendmessage.h"
#include <QTime>
#include <QDebug>

ResendMessage::ResendMessage(unsigned int againNum, unsigned int timeout,
	QByteArray arrData, QByteArray arrAgainData, int messageid, _DeviceStatus initStatus, bool bauto, QObject *parent)
	: QThread(parent)
{
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
	//�ȴ��߳�ֹͣ
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
	//���һ�η������ط����ݻ᲻ͬ�����ȷ���һ��
	QTime time;
	time.start();
	unsigned int index = 0;
	emit sigSendMessage(m_arrData);
	while (!m_bStop) {
		//��ʱ�ط�
		msleep(10);
		if(m_unTimeout > time.elapsed()) continue;
		index++;
		if (index > m_unAgainNumber) {
			//�����ط�����
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
