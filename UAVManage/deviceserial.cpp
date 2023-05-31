#include "deviceserial.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

#define _UAVPID_ 60000
#define _SerialStart_ "qz+"
#define _SerialEnd_   "\r\n"
#define _SerialOk_ "ok"
//最长20个英文字符
#define _DataMaxLen_ 20
SerialThread::SerialThread(QSerialPort* ser, QObject* parent /*= nullptr*/)
{
	m_pSerial = ser;
}

void SerialThread::clear()
{
	m_pSerial = nullptr;
}

void SerialThread::onDataSendWork(const QByteArray data)
{
	if (!m_pSerial) return;
	m_pSerial->write(data);
	m_pSerial->waitForBytesWritten(3000);
}

void SerialThread::onDataReciveWork()
{
	//有可能含有中文字符
	if (!m_pSerial) return;
	QString data = QString::fromLocal8Bit(m_pSerial->readAll());
	emit sendResultToGui(data.toLocal8Bit());
	//m_qstrData.append(data);
	//QString qstrEnd = _SerialEnd_;
	//while (m_qstrData.contains(qstrEnd)){
	//	int index = m_qstrData.indexOf(qstrEnd);
	//	QString temp = m_qstrData.left(index + qstrEnd.length());
	//	m_qstrData = m_qstrData.remove(0, temp.length());
	//	emit sendResultToGui(temp.toLocal8Bit());
	//}
}

DeviceSerial::DeviceSerial(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;

	QRegExp regExp1("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	ui.lineEditIP->setValidator(new QRegExpValidator(regExp1, this));

	m_pSerial = new SerialThread(&m_serialPort);
	(&m_serialPort)->moveToThread(&m_thread);
	m_pSerial->moveToThread(&m_thread);
	connect(&m_thread, &QThread::finished, m_pSerial, &QObject::deleteLater);           // 线程结束，自动删除对象
	connect(this, &DeviceSerial::serialDataSend, m_pSerial, &SerialThread::onDataSendWork);   // 主线程串口数据发送的信号
	connect(&m_serialPort, &QSerialPort::readyRead, m_pSerial, &SerialThread::onDataReciveWork); // 主线程通知子线程接收数据的信号
	connect(m_pSerial, &SerialThread::sendResultToGui, this, &DeviceSerial::onSerialData);              // 主线程收到数据结果的信号
	m_thread.start();
	connect(ui.btnSet, &QAbstractButton::clicked, this, &DeviceSerial::onBtnWrite);
	connect(ui.btnUpdate, &QAbstractButton::clicked, this, &DeviceSerial::onBtnRead);
	connect(ui.btnSerial, &QAbstractButton::clicked, this, &DeviceSerial::onBtnSerial);
	connect(ui.lineEditName, &QLineEdit::textEdited, this, &DeviceSerial::onLineEditChanged);
	connect(ui.lineEditPass, &QLineEdit::textEdited, this, &DeviceSerial::onLineEditChanged);

	connect(ui.btnCheckFirmware, &QAbstractButton::clicked, this, &DeviceSerial::onBtnCheckFirmware);
	connect(ui.btnManualFirmware, &QAbstractButton::clicked, this, &DeviceSerial::onBtnManualFirmware);
	connect(&m_timeoutWrite, &QTimer::timeout, this, &DeviceSerial::onTimeoutWrite);

	ui.widgetDeviceParam->setEnabled(false);
	//监控串口插拔
	m_qextSerial.setUpNotifications();
	connect(&m_qextSerial, SIGNAL(deviceDiscovered(const QextPortInfo&)), this, SLOT(onDeviceDiscovered(const QextPortInfo&)));
	connect(&m_qextSerial, SIGNAL(deviceRemoved(const QextPortInfo&)), this, SLOT(onDeviceRemoved(const QextPortInfo&)));

	foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
		quint16 pid = info.productIdentifier();
		qDebug() << info.portName() << pid;
		//无人机设备PID值
		if (_UAVPID_ != pid) continue;
		ui.comboBoxCom->addItem(info.portName());
	}
}

DeviceSerial::~DeviceSerial()
{
	if (m_timeoutWrite.isActive()) m_timeoutWrite.stop();
	if (m_pSerial) {
		m_pSerial->clear();
		m_pSerial->deleteLater();
		m_pSerial = nullptr;
	}
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
	emit sigDeviceEnabled(ui.comboBoxCom->count());
}

bool DeviceSerial::isSerialEnabled()
{
	return ui.comboBoxCom->count();
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
			if (key.contains("qz+w")){
				//写入网络配置
				m_bWriteFinished = true;
				if (_SerialOk_ == msg) {
					QMessageBox::information(this, tr("成功"), tr("设置完成"));
					return;
				}
				else {
					QMessageBox::warning(this, tr("警告"), tr("设置失败") + msg);
					return;
				}
			}
			else if (key.contains("qz+r")) {
				//读取网络配置
				QStringList list = msg.split("+");
				if (list.count() < 3) {
					QMessageBox::warning(this, tr("警告"), tr("内容错误无法刷新") + msg);
					return;
				}
				ui.lineEditIP->setText(list.at(0));
				ui.lineEditName->setText(list.at(1));
				ui.lineEditPass->setText(list.at(2));
			}
			else if (key.contains("qz+v")) {
				//固件版本
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
	if (name.toLocal8Bit().length() > _DataMaxLen_) {
		QMessageBox::warning(this, tr("警告"), tr("WIFI名称长度超出20个字母"));
		return;
	}
	if (password.toLocal8Bit().length() > _DataMaxLen_) {
		QMessageBox::warning(this, tr("警告"), tr("IFI密码长度超出20个字母"));
		return;
	}
	m_timeoutWrite.start(3 * 1000);
	m_bWriteFinished = false;
	QString text = QString("qz+w:%1+%2+%3%4").arg(ip).arg(name).arg(password).arg(_SerialEnd_);
	emit serialDataSend(text.toLocal8Bit());

	//emit serialDataSend("qz+w+name:" + name.toLocal8Bit() + QByteArray(_SerialEnd_));
	//emit serialDataSend("qz+w+password:" + password.toLocal8Bit() + QByteArray(_SerialEnd_));
}

void DeviceSerial::onBtnRead()
{
	ui.lineEditIP->clear();
	ui.lineEditName->clear();
	ui.lineEditPass->clear();
	emit serialDataSend("qz+r:" + QByteArray(_SerialEnd_));
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
		ui.widgetDeviceParam->setEnabled(false);
	}
	else {
		QString qstrCom = ui.comboBoxCom->currentText();
		m_serialPort.setPortName(qstrCom);
		m_serialPort.setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);//设置波特率和读写方向
		if (!m_serialPort.open(QIODevice::ReadWrite)) {
			QMessageBox::warning(this, tr("提示"), tr("设备无法连接，请重试"));
			return;
		}
		ui.btnSerial->setText(tr("断开"));
		ui.comboBoxCom->setEnabled(false);
		ui.widgetDeviceParam->setEnabled(true);
		//串口连接成功后读取配置内容
		onBtnRead();
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

void DeviceSerial::onBtnCheckFirmware()
{
	//通过网络获取最新版本固件
	QMessageBox::information(this, tr("提示"), tr("功能开发中"));
}

void DeviceSerial::onBtnManualFirmware()
{
	//选择本地已存在固件文件
	QMessageBox::information(this, tr("提示"), tr("功能开发中"));
}

void DeviceSerial::onLineEditChanged(QString text)
{
	QLineEdit* pLine = dynamic_cast<QLineEdit*>(sender());
	if (!pLine) return;
	QByteArray data = text.toLocal8Bit();
	int len = data.length();
	if (len > _DataMaxLen_){
		QByteArray temp = data.left(_DataMaxLen_);
		pLine->setText(QString::fromLocal8Bit(temp));
		return;
	}
	//+在传输协议中使用
	if (text.contains("+")) {
		text = text.remove("+");
		pLine->setText(text);
	}
}

void DeviceSerial::onTimeoutWrite()
{
	if (m_timeoutWrite.isActive()) m_timeoutWrite.stop();
	if (m_bWriteFinished) return;
	QMessageBox::warning(this, tr("错误"), tr("设置时无人机没有响应，请检查连接"));
}

void DeviceSerial::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	QWidget* pWidget = this;
	while (true) {
		QWidget* pTemp = dynamic_cast<QWidget*>(pWidget->parent());
		if (!pTemp) break;
		pWidget = pTemp;
	}
	m_pLabelBackground = new QLabel(pWidget);
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(pWidget->size());
	m_pLabelBackground->show();
	if (false == m_serialPort.isOpen()) {
		ui.btnSerial->setText(tr("连接中"));
		QTimer::singleShot(1000, [this]() { 
			//延时连接防止界面阻塞
			updateSerial(); 
			if (ui.comboBoxCom->count() > 0 && false == m_serialPort.isOpen()) {
				ui.btnSerial->clicked();
			}
			});
	}
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

