#pragma once

#include <QWidget>
#include "ui_devicedebug.h"

class DeviceDebug : public QDialog
{
	Q_OBJECT

public:
	DeviceDebug(QString ip, QString name, QWidget *parent = Q_NULLPTR);
	~DeviceDebug();
	
	void setName(QString name);
	void setIp(QString ip);
	void setLocalPosition(unsigned int time_boot_ms, float x, float y, float z);
protected:
	void closeEvent(QCloseEvent* event);
public slots:
	void onConnectStatus(QString name, QString ip, bool connect);
	void onDeviceMessage(QByteArray arrData, bool bReceive);
	void onMessageData(QByteArray arrData);
	void onSetBatteryStatus(float voltages, float battery, unsigned short electric);
	void onUpdateHighresImu(unsigned long long time, QList<float> list);
	void onUpdateAttitude(unsigned int time, float roll, float pitch, float yaw);
	void onUpdateLocalPosition(unsigned int time_boot_ms, float x, float y, float z);
private:
	Ui::DeviceDebug ui;
};