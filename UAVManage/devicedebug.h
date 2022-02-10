#pragma once

#include <QWidget>
#include "ui_devicedebug.h"

class DeviceDebug : public QDialog
{
	Q_OBJECT

public:
	DeviceDebug(QWidget *parent = Q_NULLPTR);
	~DeviceDebug();
	void setBatteryStatus(float voltages, float battery, unsigned short electric);
	void setLocalPosition(unsigned int time_boot_ms, float x, float y, float z);
protected:
	void closeEvent(QCloseEvent* event);
private slots:
	void onDeviceState(bool state);
	void onDeviceMessage(QByteArray arrData, bool bReceive);
	void onMessageData(QByteArray arrData);
	
	void on_btnStop_clicked();
	void on_btnTest_clicked();
	void on_btnConnect_clicked();
	void on_btnSendHex_clicked();
	void on_btnFly_clicked();
	void on_btnDown_clicked();
	void on_btnSetMode_clicked();
	void on_btnWaypoint_clicked();
	void on_btnMessageClear_clicked();
	void onCheckBoxbHeartbeat(int state);
	void onNavWaypointMessage(QString qstrText);

	void onUpdateHighresImu(unsigned long long time, QList<float> list);
	void onUpdateAttitude(unsigned int time, float roll, float pitch, float yaw);

private:
	Ui::DeviceDebug ui;
};
