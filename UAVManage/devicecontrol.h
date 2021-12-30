#pragma once

#include <QWidget>
#include "ui_devicecontrol.h"
#include <QMenu>
#include <QAction>

class DeviceControl : public QWidget
{
	Q_OBJECT

public:
	DeviceControl(QString name, QString ip = "", QWidget *parent = Q_NULLPTR);
	~DeviceControl();
	QString getName();
	void setName(QString name);
	QString getIP();
	void setIp(QString ip);
	bool connectDevice();
	void disconnectDevice();
private:
	Ui::DeviceControl ui;
	QString m_qstrIP;
};
