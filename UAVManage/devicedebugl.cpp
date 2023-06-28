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
	if (connect) ui.textBrowser->append("<font color=#0000FF>" + qstrText + tr("连接成功") + "</font>");
	else ui.textBrowser->append("<font color=#FF0000>"+qstrText + tr("连接断开")+"</font>");
}

void DeviceDebug::onDeviceMessage(QByteArray arrData, bool bReceive, int msgID)
{
	if (arrData.isEmpty()) return;
	bool bHeartbeat = false;
	if (arrData.length() >= 5 && 0 == QString(arrData.at(5))){
		bHeartbeat = true;
	}
	//不显示心跳数据
	if (bHeartbeat) return;
	if (44 != msgID && 47 != msgID && 77 != msgID && 73 != msgID) return;
	if (Qt::Checked != ui.checkWaypoint->checkState() && (44 == msgID || 47 == msgID || 73 == msgID)) {
		return;
	}
	else if (Qt::Checked != ui.checkCommand->checkState() && 77 == msgID) {
		return;
	}
	QString qstrText = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]");
	QString qstrHex = arrData.toHex().toUpper();
	QString qstrTemp;
	if (bReceive) {
		ui.textBrowser->append(QString("<font color=#000000>%1</font>").arg(qstrText + tr("接收:")));
		ui.textBrowser->append(QString("<font color=#FF6347>%1</font>").arg(qstrHex));
	}
	else {
		ui.textBrowser->append(QString("<font color=#000000>%1</font>").arg(qstrText + tr("发送:")));
		ui.textBrowser->append(QString("<font color=#1E90FF>%1</font>").arg(qstrHex));
	}
}

void DeviceDebug::onMessageData(QString qstrData)
{
	if (qstrData.isEmpty()) return;
	if (Qt::Checked != ui.checkLog->checkState()) return;
	QString qstrText = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]");
	qDebug() << ui.lineEditName->text() << qstrData;
	ui.textBrowser->append(QString("<font color=#000000>%1:%2</font>").arg(qstrText).arg(qstrData));
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

void DeviceDebug::onUpdateLocalPosition(unsigned int time_boot_ms, int x, int y, int z)
{
	ui.lineEditTime->setText(QString("%1").arg(time_boot_ms / 1000.0));
	ui.lineEditX->setText(QString("%1").arg(x));
	ui.lineEditY->setText(QString("%1").arg(y));
	ui.lineEditZ->setText(QString("%1").arg(z));
}

void DeviceDebug::closeEvent(QCloseEvent* event)
{
}
