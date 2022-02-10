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

DeviceDebug::DeviceDebug(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.btnConnect->setVisible(false);
	ui.lineEditIp->setVisible(false);
	ui.lineEditPort->setVisible(false);
	ui.btnWaypoint->setVisible(false);
	
	connect(ui.checkBoxbHeartbeat, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxbHeartbeat(int)));
	//connect(ThreadMavlink::getInstance(), SIGNAL(sigConnectState(bool)), this, SLOT(onDeviceState(bool)));
	//connect(ThreadMavlink::getInstance(), SIGNAL(sigMessageByte(QByteArray,bool)), this, SLOT(onDeviceMessage(QByteArray, bool)));
	//connect(ThreadMavlink::getInstance(), SIGNAL(sigNavWaypoint(QString)), this, SLOT(onNavWaypointMessage(QString)));
	qRegisterMetaType<QList<float>>("QList<float>");
	//connect(ThreadMavlink::getInstance(), SIGNAL(sigHighresImu(unsigned long long, QList<float>)), this, SLOT(onUpdateHighresImu(unsigned long long, QList<float>)));
	//connect(ThreadMavlink::getInstance(), SIGNAL(sigAttitude(unsigned int, float, float, float)), this, SLOT(onUpdateAttitude(unsigned int, float, float, float)));
	//connect(ThreadMavlink::getInstance(), SIGNAL(sigMessageData(QByteArray)), this, SLOT(onMessageData(QByteArray)));
}

DeviceDebug::~DeviceDebug()
{
}

void DeviceDebug::setBatteryStatus(float voltages, float battery, unsigned short electric)
{
	ui.lineEditVoltages->setText(QString("%1").arg(voltages));
	ui.lineEditBattery->setText(QString("%1").arg(battery));
	ui.lineEditElectric->setText(QString("%1 %").arg(electric));
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

void DeviceDebug::onDeviceState(bool state)
{
	if (state) {
		ui.textBrowser->append("连接成功");
		ui.btnConnect->setText("断开");
	}
	else {
		ui.textBrowser->append("连接断开");
		ui.btnConnect->setText("连接");
	}
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
		QString qstrText = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
		ui.textBrowser->append(QString("[<font color=black>%1</font>] : ").arg(qstrText));
		QString qstrHex = arrData.toHex().toUpper();
		QString qstrTemp;
		if(bReceive) qstrTemp = QString("<font color=green>%1</font>").arg(qstrHex);
		else qstrTemp = QString("<font color=blue>%1</font>").arg(qstrHex);
		ui.textBrowser->append(qstrTemp);
	}
}

void DeviceDebug::onMessageData(QByteArray arrData)
{
	ui.textBrowser->append(QString::fromLocal8Bit(arrData.data()));
}

void DeviceDebug::on_btnStop_clicked()
{
	//急停

}

void DeviceDebug::on_btnTest_clicked()
{

}

void DeviceDebug::on_btnConnect_clicked()
{

}

void DeviceDebug::on_btnSendHex_clicked()
{

}

void DeviceDebug::on_btnFly_clicked()
{

}

void DeviceDebug::on_btnDown_clicked()
{

}

void DeviceDebug::on_btnSetMode_clicked()
{

}

void DeviceDebug::on_btnWaypoint_clicked()
{
	
}

void DeviceDebug::on_btnMessageClear_clicked()
{
	ui.textBrowser->clear();
}

void DeviceDebug::onCheckBoxbHeartbeat(int state)
{
	
}

void DeviceDebug::onNavWaypointMessage(QString qstrText)
{
	qDebug() << QString("[%1]:%2").arg(QTime::currentTime().toString("hh:mm:ss.zzz")).arg(qstrText);
	ui.textBrowser->append(qstrText);
}

void DeviceDebug::onUpdateHighresImu(unsigned long long time, QList<float> list)
{
	ui.imu_time->setText(QString("%1").arg(time/ 1000000.0));
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
