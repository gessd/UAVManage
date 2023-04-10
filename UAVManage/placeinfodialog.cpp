#include "placeinfodialog.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <QTimer>

PlaceInfoDialog::PlaceInfoDialog(QPoint place, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	m_bSetNLINK = false;
	m_stationStatus = 0;
	m_nOnekeySetIndex = -1;
	m_pointPlace = place;
	connect(ui.btnComOpen, SIGNAL(clicked()), this, SLOT(onBtnOpenClicked()));
	connect(ui.btnRead, SIGNAL(clicked()), this, SLOT(onBtnReadClicked()));
	connect(ui.btnWrite, SIGNAL(clicked()), this, SLOT(onBtnWriteClicked()));
	connect(ui.btnOnekey, SIGNAL(clicked()), this, SLOT(onBtnOnekeyClicked()));

	SerialWorker* pSerialWorker = new SerialWorker(&m_serialPort);
	(&m_serialPort)->moveToThread(&m_threadSerial);
	pSerialWorker->moveToThread(&m_threadSerial);

	connect(&m_threadSerial, &QThread::finished, pSerialWorker, &QObject::deleteLater);           // 线程结束，自动删除对象
	connect(this, &PlaceInfoDialog::serialDataSend, pSerialWorker, &SerialWorker::onDataSendWork);   // 主线程串口数据发送的信号
	connect(&m_serialPort, &QSerialPort::readyRead, pSerialWorker, &SerialWorker::onDataReciveWork); // 主线程通知子线程接收数据的信号
	connect(pSerialWorker, &SerialWorker::sendResultToGui,this, &PlaceInfoDialog::onParseSettingFrame);              // 主线程收到数据结果的信号
	m_threadSerial.start();                   // 线程开始运行
	ui.labelSpace->setText(QString("场地大小:%1 X %2米").arg(place.x() / 100).arg(place.y() / 100));
}

PlaceInfoDialog::~PlaceInfoDialog()
{
	if (m_threadSerial.isRunning()) {
		m_threadSerial.exit();
		m_threadSerial.wait();
	}
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
}

QMap<QString, QPoint> PlaceInfoDialog::getStationAddress()
{
	QMap<QString, QPoint> stations;
	stations.insert("A0", QPoint(ui.tableWidget->item(0, 1)->text().toDouble() * 100, ui.tableWidget->item(0, 0)->text().toDouble() * 100));
	stations.insert("A1", QPoint(ui.tableWidget->item(1, 1)->text().toDouble() * 100, ui.tableWidget->item(1, 0)->text().toDouble() * 100));
	stations.insert("A2", QPoint(ui.tableWidget->item(2, 1)->text().toDouble() * 100, ui.tableWidget->item(2, 0)->text().toDouble() * 100));
	stations.insert("A3", QPoint(ui.tableWidget->item(3, 1)->text().toDouble() * 100, ui.tableWidget->item(3, 0)->text().toDouble() * 100));
	stations.insert("A4", QPoint(ui.tableWidget->item(4, 1)->text().toDouble() * 100, ui.tableWidget->item(4, 0)->text().toDouble() * 100));
	stations.insert("A5", QPoint(ui.tableWidget->item(5, 1)->text().toDouble() * 100, ui.tableWidget->item(5, 0)->text().toDouble() * 100));
	return stations;
}

bool PlaceInfoDialog::isValidStation()
{
	if (1 == m_stationStatus) return true;
	return false;
}

void PlaceInfoDialog::showEvent(QShowEvent* event) 
{
	ui.btnComOpen->setText(tr("连接"));
	ui.comboBox_role->setCurrentIndex(1);
	ui.lineEdit_ID->clear();
	int row = ui.tableWidget->rowCount();
	int column = ui.tableWidget->columnCount();
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			ui.tableWidget->setItem(i, j, new QTableWidgetItem(""));
		}
	}
	if (m_serialPort.isOpen()) {
		m_serialPort.close();
	}
	ui.comboBoxCom->clear();
	foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
		quint16 pid = info.productIdentifier();
		//基站设备PID值
		if (21972 != pid && 60000 != pid) continue;
		ui.comboBoxCom->addItem(info.portName());
	}
	if (1 == ui.comboBoxCom->count()) {
		//如果当前有可用串口则自动连接
		//延时是为了让窗口显示完成后再连接，防止阻塞
		QTimer::singleShot(500, [this]() { ui.btnComOpen->click();});
	}

	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->show();
}

void PlaceInfoDialog::closeEvent(QCloseEvent* event) 
{
	m_bSetNLINK = false;
	if (m_serialPort.isOpen()) {
		m_serialPort.close();
	}
}

void PlaceInfoDialog::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}

void PlaceInfoDialog::onBtnOpenClicked()
{
	m_nOnekeySetIndex = 0;
	if (m_serialPort.isOpen()) {
		m_serialPort.close();
		ui.btnComOpen->setText(tr("连接"));
	}
	else {
		QString qstrCom = ui.comboBoxCom->currentText();
		m_serialPort.setPortName(qstrCom);
		m_serialPort.setBaudRate(921600, QSerialPort::AllDirections);//设置波特率和读写方向
		if (!m_serialPort.open(QIODevice::ReadWrite)) {
			QMessageBox::warning(this, tr("提示"), tr("串口打开失败"));
			return;
		}
		ui.btnComOpen->setText(tr("断开"));
	}
	onBtnReadClicked();
}

void PlaceInfoDialog::onBtnReadClicked() 
{
	if (!m_serialPort.isOpen()) return;
	m_nOnekeySetIndex = 0;
	emit serialDataSend(_Get_Setting_Frame0_);
}

void PlaceInfoDialog::onBtnWriteClicked() 
{
	if (!m_serialPort.isOpen()) return;
	m_nOnekeySetIndex = 0;
	m_bSetNLINK = true;
	emit serialDataSend(_Get_Setting_Frame0_);
}

void PlaceInfoDialog::onBtnOnekeyClicked() 
{
	m_nOnekeySetIndex = 0;
	if (!m_serialPort.isOpen()) return;
	m_bOnekeySetNLINK = true;
	m_nOnekeySetIndex = 1;
	ui.labelStationSpace->clear();
	qDebug() << "开始一键标定";
	emit serialDataSend(_OneKey_Set_Start_);
}
//NLINK数据转数值,根据协议转换
double NLINK_ParseInt24(uint8_t byte[3])
{
	int n = (int32_t)(byte[0] << 8 | byte[1] << 16 | byte[2] << 24) / 256;
	return n / 1000.0f;
}
//数值转NLINK通信数据,根据协议转换
QByteArray NLINK_FloatToData(float f)
{
	int n = f * 1000 * 256;
	char b[] = { n >> 8, n >> 16,  n >> 24 };
	QByteArray arrData(b, 3);
	return arrData;
}
//获取校验和
uint8_t getCheckSum(QByteArray arrData)
{
	const void* data = arrData.data();
	size_t data_length = arrData.length();
	const uint8_t* bytec = (uint8_t*)data;
	uint8_t sum = 0;
	for (size_t i = 0; i < data_length; ++i) {
		sum += bytec[i];
	}
	return sum;
}

void PlaceInfoDialog::onParseSettingFrame(QByteArray arrNLINKData)
{
	//qDebug() << "----nlinkmsg:" << arrNLINKData.length()<<arrNLINKData.toHex().toUpper();
	QByteArray arrData = arrNLINKData;
	if (m_bSetNLINK) {
		m_bSetNLINK = false;
		if (!m_bOnekeySetNLINK) {
			int role = ui.comboBox_role->currentIndex() + 1;
			int id = ui.lineEdit_ID->text().trimmed().toInt();
			//int frequency = ui.lineEdit_frequency->text().trimmed().toInt();
			QByteArray arrRole = QByteArray::fromHex(QByteArray::number(role, 16));
			QByteArray arrID = QByteArray::fromHex(QByteArray::number(id, 16));
			//QByteArray arrFre = QByteArray::fromHex(QByteArray::number(frequency, 16));
			arrData[_role] = arrRole[0];
			arrData[_id] = arrID[0];
			//arrData[_reserved2] = arrFre[0];  //更新速率
			for (int row = 0; row < 9; row++) {
				for (int column = 0; column < 3; column++) {
					float number = ui.tableWidget->item(row, column)->text().toFloat();
					QByteArray arrNumber = NLINK_FloatToData(number);
					int index = (row * 9) + _xyz0 + column * 3;
					arrData[index] = arrNumber[0];
					arrData[index + 1] = arrNumber[1];
					arrData[index + 2] = arrNumber[2];
				}
			}
		}
		m_bOnekeySetNLINK = false;
		arrData[_mix] = 0x00;
		arrData[_group] = 0x00;
		//qDebug() << "----write:" << arrData.toHex().toUpper();
		//重新计数校验位
		QByteArray arrNew = arrData.left(arrData.length() - 1);
		arrNew.append(getCheckSum(arrNew));
		emit serialDataSend(arrNew);
		QMessageBox::information(this, tr("提示"), tr("数据写入完成"));
	}
	else {
		//更新界面设置
		//qDebug() << "界面更新" << QThread::currentThreadId();
		int role = QByteArray(1, arrData[_role]).toHex().toInt(0, 16);
		int id = QByteArray(1, arrData[_id]).toHex().toInt(0, 16);
		int fre = QByteArray(1, arrData[_reserved2]).toHex().toInt(0, 16);
		//[10|OFF] [11|ON]
		int led = QByteArray(1, arrData[14]).toHex().toInt(0, 16);
		ui.comboBox_role->setCurrentIndex(role - 1);
		ui.lineEdit_ID->setText(QString::number(id));
		double xmax = 0;
		double ymax = 0;
		bool bUsable = true;
		for (int row = 0; row < 10; row++) {
			for (int column = 0; column < 9; column += 3) {
				int index = (row * 9) + _xyz0 + column;
				unsigned char temp[3] = { arrData[index], arrData[index + 1], arrData[index + 2] };
				double number = NLINK_ParseInt24(temp);
				int n = column / 3;
				ui.tableWidget->setItem(row, n, new QTableWidgetItem(QString::number(number)));
				QString text = QString("第%1行 第%2列 ").arg(row).arg(n).arg(number);
				//qDebug() << "更新数据" << text;
				if (1 == n) xmax = qMax(xmax, number);
				if (0 == n) ymax = qMax(ymax, number);
				if (0 == row && 0 != number) {
					bUsable = false;
				}
				if (_InvalidValue_ == number && row <6) {
					bUsable = false;
				}
			}
		}
		if (bUsable) {
			ui.labelStationSpace->setText(QString("基站范围:%1 X %2米").arg(xmax).arg(ymax));
			if (0 == m_nOnekeySetIndex) onComparePlace(QPoint(xmax * 100, ymax * 100));
		}
		else {
			ui.labelStationSpace->setText(QString("基站范围:基站位置或数量错误"));
		}
	}

	if (m_nOnekeySetIndex > 0) {
		if (m_nOnekeySetIndex > 300) {
			//一键标定完成
			qDebug() << "标定完成";
			m_nOnekeySetIndex = 0;
			//检查标定基站位置 -8388为无效值
			double xmax = 0;
			double ymax = 0;
			for (int row = 0; row < 6; row++) {
				//暂时检查前6行数值
				for (int column = 0; column < 2; column ++) {
					double value = ui.tableWidget->item(row, column)->text().toDouble();
					if (0 == row && 0 != value) {
						//第一行必须全为0
						QMessageBox::warning(this, tr("提示"), tr("A0基站标定失败，请重试"));
						return;
					}
					if (_InvalidValue_ == value) {
						QMessageBox::warning(this, tr("提示"), QString("A%1基站标定失败，请重试").arg(row));
						return;
					}
					if (1 == column) xmax = qMax(xmax, value);
					if (0 == column) ymax = qMax(ymax, value);
				}
			}
			ui.labelStationSpace->setText(QString("基站范围:%1米 X %2米").arg(xmax).arg(ymax));
			//判断场地是否太小或太大
			if (xmax > 100 || ymax > 100) {
				QMessageBox::warning(this, tr("提示"), tr("基站范围距离太远，请检查后重新标定"));
				return;
			}
			if (xmax < 5 || ymax < 5) {
				QMessageBox::warning(this, tr("提示"), tr("基站范围距离太近，请检查后重新标定"));
				return;
			}
			if (xmax < m_pointPlace.x()) {
				QMessageBox::warning(this, tr("提示"), tr("X轴方向比项目设定场地小"));
				return;
			}
			if (ymax < m_pointPlace.y()) {
				QMessageBox::warning(this, tr("提示"), tr("Y轴方向比项目设定场地小"));
				return;
			}
			onComparePlace(QPoint(xmax * 100, ymax * 100));
			m_stationStatus = 1;
			return;
		}
		emit serialDataSend(_OneKey_Set_Next_);
		m_nOnekeySetIndex++;
		ui.progressBar->setValue(m_nOnekeySetIndex);
		//qDebug() << "标定进度" << m_nOnekeySetIndex;
	}
}

void PlaceInfoDialog::onComparePlace(QPoint point)
{
	if (0 == point.x() || 0 == point.y()) return;
	if (m_pointPlace == point) return;
	m_stationStatus = 0;
}

SerialWorker::SerialWorker(QSerialPort* ser, QObject* parent /*= nullptr*/)
{
	m_pSerial = ser;
}

void SerialWorker::onDataSendWork(const QByteArray data)
{
	// 发送数据
	m_pSerial->write(data);
	m_pSerial->waitForBytesWritten(3000);
}

void SerialWorker::onDataReciveWork()
{
	QByteArray arrData = m_pSerial->readAll();
	//只处理设置消息
	if (!m_SerialData.isEmpty()) {
		m_SerialData.append(arrData);
	}
	if (0x54 == arrData.at(0) && 0x00 == arrData.at(1)) {
		//if (arrData.length() < _ByteCount) bComplete = false;
		m_SerialData.append(arrData);
	}
	if (m_SerialData.length() >= _ByteCount) {
		m_SerialData = m_SerialData.left(_ByteCount);
		emit sendResultToGui(m_SerialData);
		m_SerialData.clear();
	}
}
