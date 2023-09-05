#include "uwbstationdata.h"
#include "definesetting.h"
#include <QDebug>
#include <QSerialPortInfo>
#include "uwb/nlink_linktrack_nodeframe0.h"

#define _SendDataInterval_    10
#define _AutoConnectInterval_ 3000
UWBStationData::UWBStationData(QObject *parent)
	: QObject(parent)
{
	m_bAutoConnect = true;
	//监控串口插拔
	m_qextSerial.setUpNotifications();
	connect(&m_qextSerial, SIGNAL(deviceDiscovered(const QextPortInfo&)), this, SLOT(onDeviceAdd(const QextPortInfo&)));
	connect(&m_qextSerial, SIGNAL(deviceRemoved(const QextPortInfo&)), this, SLOT(onDeviceRemoved(const QextPortInfo&)));
	connect(&m_serialPort, &QSerialPort::readyRead, this, &UWBStationData::onSerialReadData);
	connect(&m_serialPort, &QSerialPort::errorOccurred, [this](QSerialPort::SerialPortError error) {
		//if(QSerialPort::SerialPortError::NoError != error) qWarning() << "基站串口错误" << error;
		if (QSerialPort::SerialPortError::PermissionError == error) {
			//串口被占用，无法使用
			emit sigConnectStatus(false, QString("无法打开串口%1，基站连接失败").arg(m_serialPort.portName()));
		}
		});
	connect(&m_timerAutoConnect, &QTimer::timeout, this, &UWBStationData::onTimerAutoConnect);
	connect(&m_timerAutoSend, &QTimer::timeout, this, &UWBStationData::onTimerAutoSendData);
	m_timerAutoConnect.start(_AutoConnectInterval_);
}

UWBStationData::~UWBStationData()
{
	closeSeial();
}

bool UWBStationData::openSerial()
{
	bool bAvailable = false;
	foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
		quint16 pid = info.productIdentifier();
		//基站设备PID值
		if (_UWBSeialPID_ != pid) continue;
		bAvailable = true;
	}
	if (false == bAvailable) {
		//没有可用的串口
		return false;
	}
	m_bAutoConnect = true;
	if(!m_timerAutoConnect.isActive()) m_timerAutoConnect.start(_AutoConnectInterval_);
	connectSerial();
	return m_serialPort.isOpen();
}

void UWBStationData::closeSeial()
{
	m_bAutoConnect = false;
	m_timerAutoConnect.stop();
	m_timerAutoSend.stop();
	if (m_serialPort.isOpen()) {
		m_serialPort.close();
	}
}

bool UWBStationData::isOpen()
{
	return m_serialPort.isOpen();
}

void UWBStationData::appendData(unsigned int tag, QByteArray data)
{
	if(m_serialPort.isOpen()) m_queueData.enqueue(_ReadyData(tag, data));
}

void UWBStationData::onTimerAutoConnect()
{
	if (m_serialPort.isOpen()) return;
	if (false == m_bAutoConnect) return;
	connectSerial();
}

void UWBStationData::onDeviceAdd(const QextPortInfo& info)
{
	//qDebug() << "串口插入" << info.portName << info.productID;
	if (_UWBSeialPID_ != info.productID) return;
	if (m_serialPort.isOpen()) return;
	connectSerial();
}

void UWBStationData::onDeviceRemoved(const QextPortInfo& info)
{
	//qDebug() << "串口拔出" << info.portName << info.productID;
	if (_UWBSeialPID_ != info.productID) return;
	if (false == m_serialPort.isOpen()) return;
	if (m_serialPort.portName() == info.portName) {
		qWarning() << "基站串口被拔出";
		m_serialPort.close();
		m_timerAutoSend.stop();
		emit sigConnectStatus(false, "连接基站的串口被断开");
	}
}

void UWBStationData::onTimerAutoSendData()
{
	//一次只发送一条数据
	if(m_queueData.isEmpty()) return;
	_ReadyData ready = m_queueData.dequeue();
	//数据头部加上标签ID，尾部加上分割符
	QByteArray data = QString(ready.tag).toUtf8() + ready.data + "\r\n";
	//qCritical() << "-->串口发送标签" << ready.tag << data.toHex().toUpper();
	m_serialPort.write(data);
}

void UWBStationData::onSerialReadData()
{
	//一次接收的数据可能不完整所以需要做数据缓存
	QByteArray arrData = m_serialPort.readAll();
	if (arrData.isEmpty()) return;
	//qCritical() << "<--串口数据内容" << arrData.toHex().toUpper();
	if (!m_arrLastData.isEmpty()) {
		arrData.prepend(m_arrLastData);
		//qCritical() << "处理数据" << arrData.toHex().toUpper();
	}
	while (false == arrData.isEmpty()) {
		int nHeader = arrData.indexOf(0x55);
		//先定位头部
		if (nHeader < 0) {
			//没有头部信息，无效数据
			m_arrLastData.clear();
			return;
		}
		if (arrData.count() <= (nHeader + 1)) {
			//数据不完整，无法判断具体内容，缓存后继续接收数据
			m_arrLastData = arrData;
			return;
		}
		if (0x02 != arrData.at(nHeader + 1)) {
			//不是要处理的协议数据
			m_arrLastData.clear();
			return;
		}
		if (arrData.count() < (nHeader + 4)) {
			//数据包头部不完整，缓存后重新接收
			m_arrLastData = arrData;
			return;
		}
		int nLen = arrData.at(nHeader + 2) | arrData.at(nHeader + 3) << 8;
		if (nLen <= 0) {
			//无法解析出数据包长度，说明数据有错误，删除头部信息继续判断后续数据是否有数据包头部信息
			arrData.remove(nHeader, 4);
			continue;
		}
		if ((nHeader + nLen) > arrData.count()) {
			//本次接收的数据不完整，缓存后重新接收
			m_arrLastData = arrData;
			return;
		}
		QByteArray arrFrame = arrData.mid(nHeader, nLen);
		//尝试解包
		if (!g_nlt_nodeframe0.UnpackData((unsigned char*)arrFrame.data(), arrFrame.length())) {
			//qCritical() << "xxxxUWB数据解包失败" << arrFrame.toHex().toUpper();
			//解包不成功只删除头部数据
			arrData.remove(0, nHeader + 2);
			continue;
		}
		//解包成功后删除前边的数据
		arrData.remove(0, nHeader + nLen);
		nlt_nodeframe0_result_t* result = &g_nlt_nodeframe0.result;
		if (!result)  return;
		unsigned short count = result->valid_node_count;
		QList<_ReadyData> list;
		for (int i = 0; i < count; i++) {
			nlt_nodeframe0_node_t* node = result->nodes[i];
			if (!node) continue;
			//只处理无人机标签数据
			if (LINKTRACK_ROLE_TAG != node->role) continue;
			unsigned short id = node->id;
			QByteArray temp = QByteArray((char*)node->data, node->data_length);
			//qCritical() << "UWB标签" << id << arrFrame.toHex().toUpper();
			list.append(_ReadyData(id, temp));
		}
		emit sigReceiveData(list);
		m_arrLastData = arrData;
	}
}

void UWBStationData::connectSerial()
{
	if (m_serialPort.isOpen()) return;
	foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
		quint16 pid = info.productIdentifier();
		//基站设备PID值
		if (_UWBSeialPID_ != pid) continue;
		m_arrLastData.clear();
		//qInfo() << "准备连接基站" << info.portName();
		m_serialPort.setBaudRate(921600, QSerialPort::AllDirections);//设置波特率和读写方向
		m_serialPort.setPortName(info.portName());
		bool bOpen = m_serialPort.open(QIODevice::ReadWrite);
		if (bOpen) {
			m_timerAutoSend.start(_SendDataInterval_);
			emit sigConnectStatus(true, "连接基站成功");
		}
	}
}
