#include "devicecontrol.h"
#include <QMenu>
#include <QAction>

DeviceControl::DeviceControl(QString name, QString ip, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setName(name);
	setIp(ip);
}

DeviceControl::~DeviceControl()
{
}

QString DeviceControl::getName()
{
	return ui.labelDeviceName->text().trimmed();
}

void DeviceControl::setName(QString name)
{
	ui.labelDeviceName->setText(name);
}

QString DeviceControl::getIP()
{
	return m_qstrIP;
}

void DeviceControl::setIp(QString ip)
{
	m_qstrIP = ip;
	disconnectDevice();
	if (m_qstrIP.isEmpty()) return;
	//连接设备
	connectDevice();
}

bool DeviceControl::connectDevice()
{
	return false;
}

void DeviceControl::disconnectDevice()
{
}
