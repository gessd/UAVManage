#include "deviceserial.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

#define _UAVPID_ 60000
#define _SerialStart_ "qz+"
#define _SerialEnd_   "\r\n"
#define _SerialOk_ "ok"
SerialThread::SerialThread(QSerialPort* ser, QObject* parent /*= nullptr*/)
{
	m_pSerial = ser;
}

void SerialThread::onDataSendWork(const QByteArray data)
{
	m_pSerial->write(data);
	m_pSerial->waitForBytesWritten(3000);
}

void SerialThread::onDataReciveWork()
{
	//有可能含有中文字符
	QString data = QString::fromLocal8Bit(m_pSerial->readAll());
	m_qstrData.append(data);
	QString qstrEnd = _SerialEnd_;
	while (m_qstrData.contains(qstrEnd)){
		int index = m_qstrData.indexOf(qstrEnd);
		QString temp = m_qstrData.left(index + qstrEnd.length());
		m_qstrData.remove(index, temp.length());
		emit sendResultToGui(temp.toUtf8());
	}
}

DeviceSerial::DeviceSerial(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;

	QRegExp regExp1("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	ui.lineEditIP->setValidator(new QRegExpValidator(regExp1, this));

	SerialThread* pSerial = new SerialThread(&m_serialPort);
	(&m_serialPort)->moveToThread(&m_thread);
	pSerial->moveToThread(&m_thread);
	connect(&m_thread, &QThread::finished, pSerial, &QObject::deleteLater);           // 线程结束，自动删除对象
	connect(this, &DeviceSerial::serialDataSend, pSerial, &SerialThread::onDataSendWork);   // 主线程串口数据发送的信号
	connect(&m_serialPort, &QSerialPort::readyRead, pSerial, &SerialThread::onDataReciveWork); // 主线程通知子线程接收数据的信号
	connect(pSerial, &SerialThread::sendResultToGui, this, &DeviceSerial::onSerialData);              // 主线程收到数据结果的信号
	m_thread.start();
	connect(ui.btnSet, &QAbstractButton::clicked, this, &DeviceSerial::onBtnWrite);
	connect(ui.btnUpdate, &QAbstractButton::clicked, this, &DeviceSerial::onBtnRead);
	connect(ui.btnSerial, &QAbstractButton::clicked, this, &DeviceSerial::onBtnSerial);

	//监控串口插拔
	m_qextSerial.setUpNotifications();
	connect(&m_qextSerial, SIGNAL(deviceDiscovered(const QextPortInfo&)), this, SLOT(onDeviceDiscovered(const QextPortInfo&)));
	connect(&m_qextSerial, SIGNAL(deviceRemoved(const QextPortInfo&)), this, SLOT(onDeviceRemoved(const QextPortInfo&)));
}

DeviceSerial::~DeviceSerial()
{
	if (m_thread.isRunning()) {
		m_thread.exit();
		m_thread.wait();
	}
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
}

void DeviceSerial::updateSerial()
{
	ui.comboBoxCom->clear();
	foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
		quint16 pid = info.productIdentifier();
		//无人机设备PID值
		if (_UAVPID_ != pid) continue;
		ui.comboBoxCom->addItem(info.portName());
	}
	if (ui.comboBoxCom->count() > 0 && false == m_serialPort.isOpen()) {
		ui.btnSerial->clicked();
	}
}

void DeviceSerial::onSerialData(QByteArray data)
{
	if (data.contains(_SerialStart_) && data.contains(_SerialEnd_)) {
		QString qstrData = QString::fromLocal8Bit(data);
		QStringList all = qstrData.split(_SerialEnd_);
		for (int i = 0; i < all.count(); i++) {
			QString temp = all.at(i);
			QStringList list = temp.split(":");
			if (list.count() < 2) continue;
			QString key = list.at(0);
			QString msg = list.at(1);
			if (key.contains("qz+w+ip:") || key.contains("qz+w+name:") || key.contains("qz+w+password:")){
				if (_SerialOk_ != msg) {
					QMessageBox::warning(this, tr("警告"), tr("设置失败")+msg);
					return;
				}
			}
			else if (key.contains("qz+r+ip:")) {
				ui.lineEditIP->setText(msg);
			}
			else if (key.contains("qz+r+name:")) {
				ui.lineEditName->setText(msg);
			}
			else if (key.contains("qz+r+password:")) {
				ui.lineEditPass->setText(msg);
			}
		}
	}
}

void DeviceSerial::onBtnWrite()
{
	QString ip = ui.lineEditIP->text().trimmed();
	QString name = ui.lineEditName->text().trimmed();
	QString password = ui.lineEditPass->text().trimmed();
	if (ip.isEmpty() || name.isEmpty() || password.isEmpty()) {
		QMessageBox::warning(this, tr("警告"), tr("配置内容不能为空"));
		return;
	}
	QRegExp regExp1("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	if (!regExp1.exactMatch(ip)){
		QMessageBox::warning(this, tr("警告"), tr("IP地址输入错误"));
		return;
	}
	emit serialDataSend("qz+w+ip:" + ip.toLocal8Bit() + QByteArray(_SerialEnd_));
	emit serialDataSend("qz+w+name:" + name.toLocal8Bit() + QByteArray(_SerialEnd_));
	emit serialDataSend("qz+w+password:" + password.toLocal8Bit() + QByteArray(_SerialEnd_));
}

void DeviceSerial::onBtnRead()
{
	emit serialDataSend("qz+r+ip:" + QByteArray(_SerialEnd_));
	emit serialDataSend("qz+r+name:" + QByteArray(_SerialEnd_));
	emit serialDataSend("qz+r+password:" + QByteArray(_SerialEnd_));
}

void DeviceSerial::onBtnSerial()
{
	ui.lineEditIP->clear();
	ui.lineEditName->clear();
	ui.lineEditPass->clear();
	if (m_serialPort.isOpen()) {
		ui.btnSerial->setText(tr("连接"));
		m_serialPort.close();
		ui.comboBoxCom->setEnabled(true);
	}
	else {
		QString qstrCom = ui.comboBoxCom->currentText();
		m_serialPort.setPortName(qstrCom);
		m_serialPort.setBaudRate(921600, QSerialPort::AllDirections);//设置波特率和读写方向
		if (!m_serialPort.open(QIODevice::ReadWrite)) {
			QMessageBox::warning(this, tr("提示"), tr("设备无法连接，请重试"));
			return;
		}
		ui.btnSerial->setText(tr("断开"));
		ui.comboBoxCom->setEnabled(false);
	}
}

void DeviceSerial::onDeviceDiscovered(const QextPortInfo& info)
{
	if (_UAVPID_ != info.productID) return;
	updateSerial();
}

void DeviceSerial::onDeviceRemoved(const QextPortInfo& info)
{
	if (_UAVPID_ != info.productID) return;
	if (m_serialPort.isOpen() && m_serialPort.portName() == info.portName) {
		//当前已打开串口与拔出串口相同
		ui.btnSerial->clicked();
	}
	updateSerial();
}

void DeviceSerial::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->show();
	QTimer::singleShot(1000, [this]() { updateSerial(); });
}

void DeviceSerial::closeEvent(QCloseEvent* event)
{
	if (m_serialPort.isOpen()) {
		m_serialPort.close();
		ui.btnSerial->setText(tr("连接"));
		ui.comboBoxCom->setEnabled(true);
	}
}

void DeviceSerial::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}

