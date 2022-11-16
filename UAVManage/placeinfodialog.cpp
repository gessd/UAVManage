#include "placeinfodialog.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QThread>

PlaceInfoDialog::PlaceInfoDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_bSetNLINK = false;
	m_stationStatus = 0;
	connect(ui.btnComOpen, SIGNAL(clicked()), this, SLOT(onBtnOpenClicked()));
	connect(ui.btnRead, SIGNAL(clicked()), this, SLOT(onBtnReadClicked()));
	connect(ui.btnWrite, SIGNAL(clicked()), this, SLOT(onBtnWriteClicked()));
	connect(ui.btnOnekey, SIGNAL(clicked()), this, SLOT(onBtnOnekeyClicked()));

	connect(&m_serialPort, SIGNAL(readyRead()), this, SLOT(onSerialReadyRead()));
	m_nOnekeySetIndex = -1;
}

PlaceInfoDialog::~PlaceInfoDialog()
{
}

QMap<QString, QPoint> PlaceInfoDialog::getStationAddress()
{
	QMap<QString, QPoint> stations;
	//TODO 需要确保基站标定位置可用
	if (1 != m_stationStatus) {
		//未标定或标定失败
		return stations;
	}
	stations.insert("A0", QPoint(ui.tableWidget->item(0, 0)->text().toInt() * 100, ui.tableWidget->item(0, 1)->text().toInt() * 100));
	stations.insert("A1", QPoint(ui.tableWidget->item(1, 0)->text().toInt() * 100, ui.tableWidget->item(1, 1)->text().toInt() * 100));
	stations.insert("A2", QPoint(ui.tableWidget->item(2, 0)->text().toInt() * 100, ui.tableWidget->item(2, 1)->text().toInt() * 100));
	stations.insert("A3", QPoint(ui.tableWidget->item(3, 0)->text().toInt() * 100, ui.tableWidget->item(3, 1)->text().toInt() * 100));
	stations.insert("A4", QPoint(ui.tableWidget->item(4, 0)->text().toInt() * 100, ui.tableWidget->item(4, 1)->text().toInt() * 100));
	stations.insert("A5", QPoint(ui.tableWidget->item(5, 0)->text().toInt() * 100, ui.tableWidget->item(5, 1)->text().toInt() * 100));
	return stations;
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
		if (60000 != pid) continue;
		ui.comboBoxCom->addItem(info.portName());
	}
}

void PlaceInfoDialog::closeEvent(QCloseEvent* event) 
{
	m_bSetNLINK = false;
	if (m_serialPort.isOpen()) {
		m_serialPort.close();
	}
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
	int len = m_serialPort.write(_Get_Setting_Frame0_);
	m_serialPort.waitForBytesWritten(500);
	if (len != _ByteCount) {
		QMessageBox::warning(this, tr("提示"), tr("写入数据失败"));
	}
}

void PlaceInfoDialog::onBtnWriteClicked() 
{
	if (!m_serialPort.isOpen()) return;
	m_nOnekeySetIndex = 0;
	m_bSetNLINK = true;
	int len = m_serialPort.write(_Get_Setting_Frame0_);
	m_serialPort.waitForBytesWritten(500);
	if (len != _ByteCount) {
		QMessageBox::warning(this, tr("提示"), tr("写入数据失败"));
	}
}

void PlaceInfoDialog::onBtnOnekeyClicked() 
{
	m_nOnekeySetIndex = 0;
	if (!m_serialPort.isOpen()) return;
	m_bOnekeySetNLINK = true;
	m_nOnekeySetIndex = 1;
	m_serialPort.write(_OneKey_Set_Start_);
	m_serialPort.waitForBytesWritten(500);
}
//NLINK数据转数值,根据协议转换
float NLINK_ParseInt24(uint8_t byte[3])
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

void PlaceInfoDialog::onSerialReadyRead() 
{
	static bool bComplete = true;
	static QByteArray arrCompleteData = "";
	QByteArray arrData = m_serialPort.readAll();
	//只处理设置消息
	if (!arrCompleteData.isEmpty()) {
		arrCompleteData.append(arrData);
	}
	if (0x54 == arrData.at(0) && 0x00 == arrData.at(1)) {
		//if (arrData.length() < _ByteCount) bComplete = false;
		arrCompleteData.append(arrData);
	}
	//if (!bComplete) {
	//	//数据不完整则继续接收
	//	arrCompleteData.append(arrData);
	//}
	if (arrCompleteData.length() >= _ByteCount) {
		bComplete = true;
		arrCompleteData = arrCompleteData.left(_ByteCount);
		parseSettingFrame(arrCompleteData);
		arrCompleteData.clear();
	}
}

void PlaceInfoDialog::parseSettingFrame(QByteArray arrNLINKData)
{
	qDebug() << "----nlinkmsg:" << arrNLINKData.length()<<arrNLINKData.toHex().toUpper();
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
		qDebug() << "----write:" << arrData.toHex().toUpper();
		//重新计数校验位
		QByteArray arrNew = arrData.left(arrData.length() - 1);
		arrNew.append(getCheckSum(arrNew));
		//写入新设置
		m_serialPort.write(arrNew);
		m_serialPort.waitForBytesWritten(500);
		QMessageBox::information(this, tr("提示"), tr("数据写入完成"));
	}
	else {
		//更新界面设置
		int role = QByteArray(1, arrData[_role]).toHex().toInt(0, 16);
		int id = QByteArray(1, arrData[_id]).toHex().toInt(0, 16);
		int fre = QByteArray(1, arrData[_reserved2]).toHex().toInt(0, 16);
		//[10|OFF] [11|ON]
		int led = QByteArray(1, arrData[14]).toHex().toInt(0, 16);
		ui.comboBox_role->setCurrentIndex(role - 1);
		ui.lineEdit_ID->setText(QString::number(id));
		//ui.lineEdit_frequency->setText(QString::number(fre));
		for (int row = 0; row < 10; row++) {
			for (int column = 0; column < 9; column += 3) {
				int index = (row * 9) + _xyz0 + column;
				unsigned char temp[3] = { arrData[index], arrData[index + 1], arrData[index + 2] };
				float number = NLINK_ParseInt24(temp);
				ui.tableWidget->setItem(row, column / 3, new QTableWidgetItem(QString::number(number)));
			}
		}
	}

	if (m_nOnekeySetIndex > 0) {
		m_serialPort.write(_OneKey_Set_Next_);
		m_serialPort.waitForBytesWritten(500);
		QThread::msleep(100);
		m_nOnekeySetIndex++;
		if (m_nOnekeySetIndex > 300) {
			//一键标定完成
			m_nOnekeySetIndex = 0;
			m_stationStatus = 1;
			//TODO 检查标定基站位置 -8388为无效值
			return;
		}
	}
}