#pragma once

#include <QObject>
#include <QSerialPort>
#include <QQueue>
#include <QMutex>
#include <QTimer>
#include "serial/qextserialenumerator.h"

struct _ReadyData
{
	unsigned int tag;
	QByteArray data;
	_ReadyData(unsigned int n, QByteArray d) {
		tag = n;
		data = d;
	}
};

class UWBStationData  : public QObject
{
	Q_OBJECT

public:
	UWBStationData(QObject *parent);
	~UWBStationData();
	/**
	 * @brief 打开串口
	 */
	bool openSerial();
	/**
	 * @brief 关闭串口
	 */
	void closeSeial();
	/**
	 * @brief 串口是否连接
	 */
	bool isOpen();
	/**
	 * @brief 添加数据到队列等待统一发送
	 */
	void appendData(unsigned int tag, QByteArray data);
private slots:
	/**
	 * @brief 定时自动连接串口
	 */
	void onTimerAutoConnect();
	/**
	 * @brief 新串口设备接入
	 */
	void onDeviceAdd(const QextPortInfo& info);
	/**
	 * @brief 串口设备拔出
	 */
	void onDeviceRemoved(const QextPortInfo& info);
	/**
	 * @brief 定时发送数据到基站
	 */
	void onTimerAutoSendData();
	/**
	 * @brief 串口收到的数据
	 */
	void onSerialReadData();
private:
	/**
	 * 连接串口
	 */
	void connectSerial();
signals:
	void sigConnectStatus(bool connect, QString error);
	void sigReceiveData(QList<_ReadyData>);
private:
	//串口管理
	QSerialPort m_serialPort;
	//监控串口插拔
	QextSerialEnumerator m_qextSerial;
	//是否自动连接串口
	bool m_bAutoConnect;
	//等待要发送的数据
	QQueue<_ReadyData> m_queueData;
	//定时自动连接串口
	QTimer m_timerAutoConnect;
	//定时自动发送数据
	QTimer m_timerAutoSend;
	//接收的数据
	QByteArray m_arrLastData;
};
