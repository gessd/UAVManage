#include "DeviceDebug.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QComboBox>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <qmath.h>

DeviceDebug::DeviceDebug(QString ip, QString name, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setIp(ip);
	setName(name);
	connect(ui.btnMessageClear, &QAbstractButton::clicked, [this]() {
		ui.textBrowser->clear();
		});
}

DeviceDebug::~DeviceDebug()
{
}

void DeviceDebug::setName(QString name)
{
	ui.lineEditName->setText(name);
}

void DeviceDebug::setIp(QString ip)
{
	ui.lineEditIp->setText(ip);
}

void DeviceDebug::onConnectStatus(QString name, QString ip, bool connect)
{
	QString qstrText = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]:");
	if (connect) ui.textBrowser->append(qstrText + tr("连接成功"));
	else ui.textBrowser->append(qstrText + tr("连接断开"));
}

void DeviceDebug::onDeviceMessage(QByteArray arrData, bool bReceive)
{
	if (Qt::Checked == ui.checkBoxbOnlylog->checkState()) return;
	bool bHeartbeat = false;
	if (arrData.length() >= 5) {
		if (0 == QString(arrData.at(5))) {
			bHeartbeat = true;
		}
	}
	if (!bHeartbeat) {
		QString qstrText = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]");
		QString qstrHex = arrData.toHex().toUpper();
		QString qstrTemp;
		if (bReceive) {
			ui.textBrowser->append(QString("<font color=#FFFFFF>%1</font>").arg(qstrText + tr("接收:")));
			qstrTemp = QString("<font color=#FF6347>%1</font>").arg(qstrHex);
		}
		else {
			ui.textBrowser->append(QString("<font color=#FFFFFF>%1</font>").arg(qstrText + tr("发送:")));
			qstrTemp = QString("<font color=#1E90FF>%1</font>").arg(qstrHex);
		}
		ui.textBrowser->append(qstrTemp);
	}
}

void DeviceDebug::onMessageData(QByteArray arrData)
{
	ui.textBrowser->append(QString::fromLocal8Bit(arrData.data()));
}

void DeviceDebug::onSetBatteryStatus(float voltages, float battery, unsigned short electric)
{
	ui.lineEditVoltages->setText(QString("%1").arg(voltages));
	ui.lineEditBattery->setText(QString("%1").arg(battery));
	ui.lineEditElectric->setText(QString("%1 %").arg(electric));
}

void DeviceDebug::onUpdateHighresImu(unsigned long long time, QList<float> list)
{
	ui.imu_time->setText(QString("%1").arg(time / 1000000.0));
	ui.imu_1->setText(QString::number(list.at(0), 'f', 3));
	ui.imu_2->setText(QString::number(list.at(1), 'f', 3));
	ui.imu_3->setText(QString::number(list.at(2), 'f', 3));
	ui.imu_4->setText(QString::number(list.at(3), 'f', 3));
	ui.imu_5->setText(QString::number(list.at(4), 'f', 3));
	ui.imu_6->setText(QString::number(list.at(5), 'f', 3));
	ui.imu_7->setText(QString::number(list.at(6), 'f', 3));
	ui.imu_8->setText(QString::number(list.at(7), 'f', 3));
	ui.imu_9->setText(QString::number(list.at(8), 'f', 3));
}

void DeviceDebug::onUpdateAttitude(unsigned int time, float roll, float pitch, float yaw)
{
	ui.lineEditTime->setText(QString("%1").arg(time / 1000.0));
	ui.lineEditroll->setText(QString::number(roll, 'f', 3));
	ui.lineEditpitch->setText(QString::number(pitch, 'f', 3));
	ui.lineEdityaw->setText(QString::number(yaw, 'f', 3));
}

void DeviceDebug::onUpdateLocalPosition(unsigned int time_boot_ms, float x, float y, float z)
{
	ui.lineEditTime->setText(QString("%1").arg(time_boot_ms / 1000.0));
	ui.lineEditX->setText(QString("%1").arg(x));
	ui.lineEditY->setText(QString("%1").arg(y));
	ui.lineEditZ->setText(QString("%1").arg(z));
}

void DeviceDebug::setLocalPosition(unsigned int time_boot_ms, float x, float y, float z)
{
	ui.lineEditTime->setText(QString("%1").arg(time_boot_ms/1000.0));
	ui.lineEditX->setText(QString("%1").arg(x));
	ui.lineEditY->setText(QString("%1").arg(y));
	ui.lineEditZ->setText(QString("%1").arg(z));
}

void DeviceDebug::closeEvent(QCloseEvent* event)
{
}
