#include "deviceserial.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include <QKeyEvent>
#include <QDateTime>
#include "downloadtool.h"
#include "definesetting.h"
#include "paramreadwrite.h"

#define _UAVPID_ 60000
#define _SerialStart_ "qz+"
#define _SerialEnd_   "\r\n"
#define _SerialOk_    "ok"
//最长20个英文字符
#define _DataMaxLen_ 20

#define _DisableUpdateFirmware_

DeviceSerial::DeviceSerial(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;

	QRegExp regExp1("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	ui.lineEditIP->setValidator(new QRegExpValidator(regExp1, this));

	connect(&m_serialPort, &QSerialPort::readyRead, this, &DeviceSerial::onSerialReadyRead);
	connect(ui.btnSet, &QAbstractButton::clicked, this, &DeviceSerial::onBtnWrite);
	connect(ui.btnUpdate, &QAbstractButton::clicked, this, &DeviceSerial::onBtnRead);
	connect(ui.btnSerial, &QAbstractButton::clicked, this, &DeviceSerial::onBtnSerial);
	connect(ui.lineEditName, &QLineEdit::textEdited, this, &DeviceSerial::onLineEditChanged);
	connect(ui.lineEditPass, &QLineEdit::textEdited, this, &DeviceSerial::onLineEditChanged);

	connect(ui.btnCheckFirmware, &QAbstractButton::clicked, this, &DeviceSerial::onBtnCheckFirmware);
	connect(ui.btnManualFirmware, &QAbstractButton::clicked, this, &DeviceSerial::onBtnManualFirmware);
	connect(ui.btnAutoUpdateFirmware, &QAbstractButton::clicked, this, &DeviceSerial::onBtnAutoUpdateFirmwareClicked);
	connect(ui.btnSend, &QAbstractButton::clicked, this, &DeviceSerial::onBtnSendClicekd);
	connect(ui.btnClear, &QAbstractButton::clicked, this, &DeviceSerial::onBtnClearClicked);
	connect(&m_timeoutWrite, &QTimer::timeout, this, &DeviceSerial::onTimeoutWrite);

	ui.groupBoxNetwork->setEnabled(false);
	ui.groupBoxFirmware->setEnabled(false);
	ui.groupBoxDebug->setEnabled(false);
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
	
	//固件更新使用
	m_bYmodemTransmitStatus = false;
	m_pYmodemFileTransmit = new YmodemFileTransmit(this);
	connect(m_pYmodemFileTransmit, SIGNAL(transmitProgress(int)), this, SLOT(onYmodemTransmitProgress(int)));
	connect(m_pYmodemFileTransmit, SIGNAL(transmitStatus(YmodemFileTransmit::Status)), this, SLOT(onYmodemTransmitStatus(YmodemFileTransmit::Status)));

	setFixedWidth(420);
	ui.widgetData->setVisible(false);
}

DeviceSerial::~DeviceSerial()
{
	if (m_timeoutWrite.isActive()) m_timeoutWrite.stop();
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	if (m_pYmodemFileTransmit) {
		m_pYmodemFileTransmit->deleteLater();
		m_pYmodemFileTransmit = nullptr;
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
	ui.btnSerial->setEnabled(ui.comboBoxCom->count());
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
				qInfo() << "无人机固件版本" << msg;
				msg = msg.replace("V", "");
				ui.lineEditFirmwareVersion->setText(msg);
			}
			else if (key.contains("qz+dr")) {
				//设备ID
				qInfo() << "设备ID" << msg;
				ui.lineEditID->setText(msg);
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
	QString text = QString("qz+w:%1+%2+%3").arg(ip).arg(name).arg(password);
	sendDataToSerial(text.toLocal8Bit());
}

void DeviceSerial::onBtnRead()
{
	qInfo() << "读取网络配置信息";
	ui.lineEditIP->clear();
	ui.lineEditName->clear();
	ui.lineEditPass->clear();
	sendDataToSerial("qz+r:");
}

void DeviceSerial::onBtnSerial()
{
	ui.lineEditIP->clear();
	ui.lineEditName->clear();
	ui.lineEditPass->clear();
	ui.lineEditFirmwareVersion->clear();
	if (m_serialPort.isOpen()) {
		qInfo() << "断开串口连接";
		dataRecord(true, QString("串口连接断开").toLocal8Bit());
		ui.btnSerial->setText(tr("连接"));
		m_serialPort.close();
		ui.comboBoxCom->setEnabled(true);
		ui.groupBoxNetwork->setEnabled(false);
		ui.groupBoxFirmware->setEnabled(false);
		ui.groupBoxDebug->setEnabled(false);
	}
	else {
		qInfo() << "准备连接串口";
		QString qstrCom = ui.comboBoxCom->currentText();
		if (qstrCom.isEmpty()) {
			return;
		}
		m_serialPort.setPortName(qstrCom);
		m_serialPort.setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);//设置波特率和读写方向
		if (!m_serialPort.open(QIODevice::ReadWrite)) {
			qInfo() << "连接串口失败";
			QMessageBox::warning(this, tr("提示"), tr("设备无法连接，请重试"));
			return;
		}
		qInfo() << "连接串口成功，准备读取配置信息";
		dataRecord(true, QString("串口连接成功").toLocal8Bit());
		ui.btnSerial->setText(tr("断开"));
		ui.comboBoxCom->setEnabled(false);
		ui.groupBoxNetwork->setEnabled(true);
		ui.groupBoxFirmware->setEnabled(true);
		ui.groupBoxDebug->setEnabled(true);
		//串口连接成功后读取配置内容
		onBtnRead();
		QTimer::singleShot(500, [this]() { onBtnCheckFirmware(); });
		QTimer::singleShot(1000, [this]() { on_btnReadID_clicked(); });
	}
}

void DeviceSerial::onDeviceDiscovered(const QextPortInfo& info)
{
	if (_UAVPID_ != info.productID) return;
	qDebug() << "串口插入" << info.portName;
	updateSerial();
	if (isActiveWindow() && false == m_serialPort.isOpen()) {
		onBtnSerial();
	}
}

void DeviceSerial::onDeviceRemoved(const QextPortInfo& info)
{
	if (_UAVPID_ != info.productID) return;
	qDebug() << "串口拔出" << info.portName;
	if (m_serialPort.portName() == info.portName) {
		if (m_serialPort.isOpen()) {
			qWarning() << "正在使用的串口被拔出";
			onBtnSerial();
		}
		if (m_bYmodemTransmitStatus) {
			m_pYmodemFileTransmit->stopTransmit();
			this->setEnabled(!m_bYmodemTransmitStatus);
		}
	}
	updateSerial();
}

void DeviceSerial::onBtnCheckFirmware()
{
	qInfo() << "读取固件版本信息";
	ui.lineEditFirmwareVersion->clear();
	ui.lineEditServerVersion->clear();
	sendDataToSerial("qz+v:");
	//从服务器获取最新固件版本号

	QString qstrSavePath = QApplication::applicationDirPath() + "/temp";
	QString qstrUrl = QString("%1/%2").arg(_ServerUrl_).arg(_VersionFile_);
	DownloadTool* download = new DownloadTool(qstrUrl, qstrSavePath, this);
	download->startDownload();
	connect(download, &DownloadTool::sigDownloadFinished, [this](QString error) {
		if (error.isEmpty()) {
			//解析升级配置文件，读取最新版本号
			QString config = QString("%1%2/%3").arg(QApplication::applicationDirPath()).arg("/temp").arg(_VersionFile_);
			QString qstrNewVersionNumber = ParamReadWrite::readParam("version", "", _Firmware_, config).toString();
			qDebug() << "服务器固件版本" << qstrNewVersionNumber;
			ui.lineEditServerVersion->setText(qstrNewVersionNumber);
			if (qstrNewVersionNumber.isEmpty()) return;
			QStringList list = qstrNewVersionNumber.split(".");
			if (3 != list.count()) return;
			unsigned int nNewVersion = list.at(0).toInt() * 100 * 100 + list.at(1).toInt() * 100 + list.at(2).toInt();
			QString qstrCurrentVserion = ui.lineEditFirmwareVersion->text().trimmed();
			if (qstrCurrentVserion.isEmpty()) return;
			list = qstrCurrentVserion.split(".");
			if (3 != list.count()) return;
			unsigned int nCurrentVersion = list.at(0).toInt() * 100 * 100 + list.at(1).toInt() * 100 + list.at(2).toInt();
			if (nNewVersion > nCurrentVersion) {
				ui.btnAutoUpdateFirmware->setVisible(true);
#ifndef _DisableUpdateFirmware_
				QMessageBox::StandardButton button = QMessageBox::question(this, tr("升级"), tr("有新版本固件可以升级，是否现在升级？"));
				if (QMessageBox::StandardButton::Yes == button) {
					onBtnAutoUpdateFirmwareClicked();
				}
#endif
			}
		}
		else {
			ui.lineEditServerVersion->setText("出错");
		}
		});
}

void DeviceSerial::onBtnManualFirmware()
{
	//固件更新过程
	//1.先发送指令qz+f
	//2.收到回应The number should be either 1, 2, 3 or 4
	//3.发送指令1
	//4.收到回应C
	//5.发送固件BIN文件
#ifdef _DisableUpdateFirmware_
	QMessageBox::information(this, tr("提示"), tr("功能开发中"));
	return;
#endif
	//选择本地已存在固件文件
	QString qstrFile = QFileDialog::getOpenFileName(this, tr("选择固件"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "File(*.bin)");
	if (qstrFile.isEmpty()) return;
	qInfo() << "手动选择固件文件" << qstrFile;
	m_qstrBinFile = qstrFile;
	QFileInfo info(qstrFile);
	QString name = info.fileName();
	ui.labelYmodemTransmitError->setText("准备手动更新固件文件"+name);
	ui.widgetYmodemTransmit->setVisible(true);
	sendDataToSerial("qz+f");
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

void DeviceSerial::onSerialReadyRead()
{
	if (!m_serialPort.isOpen()) return;
	QByteArray data = m_serialPort.readAll();
	qDebug() << "串口收到内容" << data;
	dataRecord(false, data);
	onSerialData(data);
	if (false == m_bYmodemTransmitStatus && data.contains("either 1, 2, 3")) {
		sendDataToSerial("1");
	} 
	if ("C" == data) {
		//发送固件文件
		if (m_bYmodemTransmitStatus) return;
		if (false == QFile::exists(m_qstrBinFile)) {
			static bool bWait = false;
			if (bWait) return;
			bWait = true;
			QMessageBox::StandardButton button = QMessageBox::question(this, tr("询问"), tr("无人机等待更新固件，是否现在更新"));
			if (QMessageBox::StandardButton::Yes != button) return;
			m_qstrBinFile = QFileDialog::getOpenFileName(this, tr("选择固件"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "File(*.bin)");
			qInfo() << "手动选择固件文件";
			if (m_qstrBinFile.isEmpty()) return;
		}
		qInfo() << "固件所在位置" << m_qstrBinFile;
		//先断开串口连接
		if (m_serialPort.isOpen()){
			onBtnSerial();
		}
		qInfo() << "开始发送固件到无人机";
		ui.labelYmodemTransmitError->setText("正在更新中");
		ui.progressBarTransmit->setValue(0);
		m_pYmodemFileTransmit->setFileName(m_qstrBinFile);
		m_pYmodemFileTransmit->setPortName(ui.comboBoxCom->currentText());
		m_pYmodemFileTransmit->setPortBaudRate(QSerialPort::Baud115200);
		if (m_pYmodemFileTransmit->startTransmit()) {
			m_bYmodemTransmitStatus = true;
		}
		else {
			m_bYmodemTransmitStatus = false;
			QMessageBox::warning(this, tr("错误"), tr("固件更新出错，无法发送固件到无人机"));
		}
		this->setEnabled(!m_bYmodemTransmitStatus);
	}
}

void DeviceSerial::onBtnSendClicekd()
{
	QString text = ui.lineEditData->text().trimmed();
	if (text.isEmpty()) return;
	sendDataToSerial(text.toLocal8Bit());
}

void DeviceSerial::onBtnClearClicked()
{
	ui.textBrowserData->clear();
}

void DeviceSerial::onBtnAutoUpdateFirmwareClicked()
{
#ifdef _DisableUpdateFirmware_
	QMessageBox::information(this, tr("提示"), tr("功能开发中"));
	return;
#endif
	//从服务器上下载固件并写入无人机
	qInfo() << "准备自动更新固件文件";
	ui.widgetYmodemTransmit->setVisible(true);
	QString qstrSavePath = QApplication::applicationDirPath() + "/temp";
	QString config = QString("%1%2/%3").arg(QApplication::applicationDirPath()).arg("/temp").arg(_VersionFile_);
	QString filename = ParamReadWrite::readParam("file", "", _Firmware_, config).toString();
	QString qstrUrl = QString("%1/%2").arg(_ServerUrl_).arg(filename);
	m_qstrBinFile = qstrSavePath + "/" + filename;
	DownloadTool* download = new DownloadTool(qstrUrl, qstrSavePath, this);
	download->startDownload();
	ui.labelYmodemTransmitError->setText("正在下载最新版本固件");
	connect(download, &DownloadTool::sigProgress, [this](qint64 bytesRead, qint64 totalBytes, qreal progress) {
		//文件下载进度
		ui.progressBarTransmit->setValue(progress * 100);
		});
	connect(download, &DownloadTool::sigDownloadFinished, [this](QString error) {
		qInfo() << "最新固件下载完成" << m_qstrBinFile;
		ui.labelYmodemTransmitError->setText("准备更新最新固件");
		ui.progressBarTransmit->setValue(0);
		//下载固件过程中有可能串口被断开，所以此处重新判断
		if (false == m_serialPort.isOpen()) {
			QMessageBox::warning(this, tr("错误"), tr("串口未连接，无法升级固件"));
			return;
		}
		sendDataToSerial("qz+f");
		});
}

void DeviceSerial::onYmodemTransmitProgress(int progress)
{
	ui.progressBarTransmit->setValue(progress);
}

void DeviceSerial::onYmodemTransmitStatus(YmodemFileTransmit::Status status)
{
	qInfo() << "固件更新状态" << status;
	switch (status){
	case YmodemFileTransmit::StatusEstablish: "未开始"; break;
	case YmodemFileTransmit::StatusTransmit: "更新中"; break;
	case YmodemFileTransmit::StatusFinish:
		m_bYmodemTransmitStatus = false;
		ui.labelYmodemTransmitError->setText("固件更新成功");
		break;
	default:
		qWarning() << "无人机固件更新失败" << status;
		m_bYmodemTransmitStatus = false;
		ui.labelYmodemTransmitError->setText("<font color=red>固件更新失败</font>");
	}
	this->setEnabled(!m_bYmodemTransmitStatus);
	if (false == m_bYmodemTransmitStatus) {
		//更新完成或失败，重新连接串口
		if(isActiveWindow()) onBtnSerial();
	}
}

void DeviceSerial::on_btnReadID_clicked()
{
	ui.lineEditID->clear();
	sendDataToSerial("qz+dr:");
}

void DeviceSerial::showEvent(QShowEvent* event)
{
	ui.lineEditServerVersion->clear();
	ui.textBrowserData->clear();
	ui.btnAutoUpdateFirmware->setVisible(false);
	ui.widgetYmodemTransmit->setVisible(false);
	setFixedWidth(420);
	ui.widgetData->setVisible(false);
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
	ui.labelYmodemTransmitError->clear();
	ui.progressBarTransmit->setValue(0);
	m_pLabelBackground = new QLabel(pWidget);
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(pWidget->size());
	m_pLabelBackground->show();
	if (false == m_serialPort.isOpen()) {
		ui.btnSerial->setText(tr("连接中"));
		QTimer::singleShot(1000, [this]() { 
			//延时连接防止界面阻塞
			updateSerial(); 
			qInfo() << "自动连接串口";
			if (ui.comboBoxCom->count() > 0 && false == m_serialPort.isOpen()) {
				onBtnSerial();
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
	m_qstrBinFile.clear();
	if (m_pLabelBackground) m_pLabelBackground->close();
	if (m_serialPort.isOpen()) {
		m_serialPort.close();
		ui.btnSerial->setText(tr("连接"));
		ui.comboBoxCom->setEnabled(true);
	}
	if (m_bYmodemTransmitStatus) {
		m_pYmodemFileTransmit->stopTransmit();
	}
}

void DeviceSerial::keyPressEvent(QKeyEvent* event)
{
	if (Qt::Key_Q == event->key()) {
		setFixedWidth(900);
		ui.widgetData->setVisible(true);
	}
}

void DeviceSerial::sendDataToSerial(const QByteArray data)
{
	if (!m_serialPort.isOpen()) return;
	QByteArray temp = data + _SerialEnd_;
	qDebug() << "串口发送数据内容" << temp;
	dataRecord(true, temp);
	m_serialPort.write(temp);
}

void DeviceSerial::dataRecord(bool send, QByteArray data)
{
	QString qstrText = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]:");
	QString color = "#000000";
	if (send) color = "#0000FF";
	QString text = QString("<font color=%1>%2</font>").arg(color).arg(QString::fromLocal8Bit(data));
	ui.textBrowserData->append(qstrText + text);
}

