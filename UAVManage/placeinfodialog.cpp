#include "placeinfodialog.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include "definesetting.h"

PlaceInfoDialog::PlaceInfoDialog(QPoint place, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	m_bSetNLINK = false;
	m_bIsUpdateStation = false;
	m_stationStatus = 0;
	m_nOnekeySetIndex = -1;
	m_pointPlace = place;
	//0x00一键标定完成
	m_cOneKeyStatus = 0x00;
	connect(ui.btnComOpen, SIGNAL(clicked()), this, SLOT(onBtnOpenClicked()));
	connect(ui.btnRead, SIGNAL(clicked()), this, SLOT(onBtnReadClicked()));
	connect(ui.btnWrite, SIGNAL(clicked()), this, SLOT(onBtnWriteClicked()));
	connect(ui.btnOnekey, SIGNAL(clicked()), this, SLOT(onBtnOnekeyClicked()));

	connect(&m_serialPort, &QSerialPort::readyRead, this, &PlaceInfoDialog::onSerialReadData);
	ui.labelSpace->setText(QString("场地大小:%1米 X %2米").arg(place.x() / 100.0).arg(place.y() / 100.0));
	ui.btnWrite->setVisible(false);
	connect(&m_timerOnekeyStatus, &QTimer::timeout, this, &PlaceInfoDialog::onTimerOnekeyStatus);
}

PlaceInfoDialog::~PlaceInfoDialog()
{
	if (m_timerOnekeyStatus.isActive()) {
		m_timerOnekeyStatus.stop();
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

bool PlaceInfoDialog::isUpdateStation()
{
	return m_bIsUpdateStation;
}

bool PlaceInfoDialog::isValidStation()
{
	if (1 == m_stationStatus) return true;
	return false;
}

void PlaceInfoDialog::showEvent(QShowEvent* event) 
{
	ui.btnComOpen->setText(tr("连接"));
	ui.comboBox_role->setCurrentIndex(2);
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
		if (_UWBSeialPID_ != pid) continue;
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
		clearStatus();
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
	sendDataToSerial(_Get_Setting_Frame0_);
}

void PlaceInfoDialog::onBtnWriteClicked() 
{
	if (!m_serialPort.isOpen()) return;
	m_nOnekeySetIndex = 0;
	m_bSetNLINK = true;
	sendDataToSerial(_Get_Setting_Frame0_);
}

void PlaceInfoDialog::onBtnOnekeyClicked() 
{
	m_nOnekeySetIndex = 0;
	if (!m_serialPort.isOpen()) return;
	m_bOnekeySetNLINK = true;
	m_nOnekeySetIndex = 1;
	ui.labelStationSpace->clear();
	ui.btnRead->setEnabled(false);
	ui.labelStationSpace->setText("正在进行一键标定");
	qInfo() << "开始一键标定";
	//先发送读取数据指令，根据返回数据修改对应字节使其变成一键标定指令然后发送
	sendDataToSerial(_Get_Setting_Frame0_);
	m_bIsUpdateStation = true;
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

void PlaceInfoDialog::onSerialReadData()
{
	QByteArray arrData = m_serialPort.readAll();
	//只处理设置消息
	if (!m_SerialData.isEmpty() && 0x54 == m_SerialData.at(0) && 0x00 == m_SerialData.at(1)) {
		m_SerialData.append(arrData);
	}
	if (0x54 == arrData.at(0) && 0x00 == arrData.at(1)) {
		m_SerialData.clear();
		m_SerialData.append(arrData);
	}
	if (m_SerialData.length() >= _ByteCount && 0x54 == m_SerialData.at(0) && 0x00 == m_SerialData.at(1)) {
		QByteArray data = m_SerialData.left(_ByteCount);
		m_SerialData.clear();
		//校验数据
		QByteArray arrNew = data.left(data.length() - 1);
		arrNew.append(getCheckSum(arrNew));
		if (data == arrNew) {
			//校验后数据与原始数据一直则处理此协议
			onParseSettingFrame(data);
		}
	}
}

void PlaceInfoDialog::onParseSettingFrame(QByteArray arrNLINKData)
{
	//qDebug() << "----nlinkmsg:" << arrNLINKData.length()<<arrNLINKData.toHex().toUpper();
	QByteArray arrData = arrNLINKData;
	m_arrLastData = arrNLINKData;
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
				//因显示时调换了X列与Y列，所有写入时再调换回来
				QByteArray a0 = NLINK_FloatToData(ui.tableWidget->item(row, 1)->text().toFloat());
				QByteArray a1 = NLINK_FloatToData(ui.tableWidget->item(row, 0)->text().toFloat());
				QByteArray a2 = NLINK_FloatToData(ui.tableWidget->item(row, 2)->text().toFloat());
				int index = (row * 9) + _xyz0 + 0 * 3;
				arrData[index] = a0[0];
				arrData[index + 1] = a0[1];
				arrData[index + 2] = a0[2];
				index = (row * 9) + _xyz0 + 1 * 3;
				arrData[index] = a1[0];
				arrData[index + 1] = a1[1];
				arrData[index + 2] = a1[2];
				index = (row * 9) + _xyz0 + 2 * 3;
				arrData[index] = a2[0];
				arrData[index + 1] = a2[1];
				arrData[index + 2] = a2[2];
			}
		}
		m_bOnekeySetNLINK = false;
		arrData[_mix] = 0x00;
		arrData[_group] = 0x00;
		//qDebug() << "----write:" << arrData.toHex().toUpper();
		//重新计数校验位
		QByteArray arrNew = arrData.left(arrData.length() - 1);
		arrNew.append(getCheckSum(arrNew));
		sendDataToSerial(arrNew);
		//QMessageBox::information(this, tr("提示"), tr("数据写入完成"));
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
				//界面显示时X列与Y列调换显示
				if (0 == n) {
					ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(number)));
					ymax = qMax(ymax, number);
					QString text = QString("第%1行 第0列 %2").arg(row).arg(number);
				}
				else if (1 == n) {
					ui.tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(number)));
					xmax = qMax(xmax, number);
					QString text = QString("第%1行 第1列 %2").arg(row).arg(number);
				}
				else if (2 == n) {
					ui.tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(number)));
					QString text = QString("第%1行 第2列 %2").arg(row).arg(number);
				}
				if (0 == row && 0 != number) {
					bUsable = false;
				}
				if (_InvalidValue_ == number && row <6) {
					bUsable = false;
				}
			}
		}
		//更新基站在坐标轴的位置
		QMap<QString, QPointF> maps;
		maps.insert("A0", QPointF(ui.tableWidget->item(0, 1)->text().toDouble(), ui.tableWidget->item(0, 0)->text().toDouble()));
		maps.insert("A1", QPointF(ui.tableWidget->item(1, 1)->text().toDouble(), ui.tableWidget->item(1, 0)->text().toDouble()));
		maps.insert("A2", QPointF(ui.tableWidget->item(2, 1)->text().toDouble(), ui.tableWidget->item(2, 0)->text().toDouble()));
		maps.insert("A3", QPointF(ui.tableWidget->item(3, 1)->text().toDouble(), ui.tableWidget->item(3, 0)->text().toDouble()));
		maps.insert("A4", QPointF(ui.tableWidget->item(4, 1)->text().toDouble(), ui.tableWidget->item(4, 0)->text().toDouble()));
		maps.insert("A5", QPointF(ui.tableWidget->item(5, 1)->text().toDouble(), ui.tableWidget->item(5, 0)->text().toDouble()));
		ui.widget->drawPoint(maps);

		if (bUsable) {
			ui.labelStationSpace->setText(QString("基站范围:%1米 X %2米").arg(xmax).arg(ymax));
		}
		else {
			ui.labelStationSpace->setText(QString("基站范围:基站位置或数量错误"));
		}
	}

	if (m_nOnekeySetIndex > 0) {
		if (1 == m_nOnekeySetIndex) {
			m_nOnekeySetIndex++;
			//m_cOneKeyStatus = arrData.at(17);
			//根据接受数据改为一键标定指令然后发送开始一键标定指令
			arrData[2] = 0x02;
			arrData[17] = 0x08;
			QByteArray arrNew = arrData.left(arrData.length() - 1);
			arrNew.append(getCheckSum(arrNew));
			sendDataToSerial(arrNew);
			//定时200毫秒查询一次标定状态
			m_timerOnekeyStatus.start(200);
			m_timerOnekeyStatus.setProperty("start", QDateTime::currentDateTime().toTime_t());
			return;
		}
		//与标定前状态一致则标定完成
		if (arrData.at(17) == 0x00) {
			verifyBaseStationPosition();
		}
	}
}

void PlaceInfoDialog::onTimerOnekeyStatus()
{
	//查询一键标定状态
	if (_ByteCount != m_arrLastData.count()) return;
	uint nStart = m_timerOnekeyStatus.property("start").toUInt();
	uint nCurrent = QDateTime::currentDateTime().toTime_t();
	if ((nCurrent - nStart) > 60) {
		//一键标定超时，停止一键标定
		m_timerOnekeyStatus.stop();
		//QMessageBox question(QMessageBox::Question, "询问", "基站长时间无法标定完成，请选择继续标定或结束标定直接使用最后的基站位置？"
		QMessageBox question(QMessageBox::Question, "当前标定时间过长，请按如下步骤检查:", "\
  1.请保持基站之间无遮挡、空旷;\n\
  2.基站需要保持在同一个平面，一般要求各个基站的高度误差不超过20cm;\n\
  3.基站布局范围一般需大于1 * 1m;\n\
  4.基站长宽比一般需小于3:1;\n\
  5.若以上步骤未解决您的问题，请点击结束标定，最后一次标定结果将会被存储在发起一键标定的基站或控制台;\n\
  6.若标定结果与实际情况大致符合，则自动标定完成;\n\
  是否结束标定?"
			, QMessageBox::Reset | QMessageBox::Apply, this);
		question.setButtonText(QMessageBox::Reset, "继续标定");
		question.setButtonText(QMessageBox::Apply, "结束标定");
		int res = question.exec();
		if (QMessageBox::Reset == res) {
			qInfo() << "基站长时间无法标定完成，继续标定";
			m_nOnekeySetIndex = 2;
			m_timerOnekeyStatus.setProperty("start", QDateTime::currentDateTime().toTime_t());
			m_timerOnekeyStatus.start(200);
		}
		else if(QMessageBox::Apply == res){
			qInfo() << "基站长时间无法标定完成，确认标定完成，直接使用基站位置";
			m_bOnekeySetNLINK = false;
			m_nOnekeySetIndex = 0;
			ui.labelStationSpace->setText("基站长时间无法标定成功");
			QByteArray arrData = m_arrLastData;
			arrData[2] = 0x02;
			arrData.remove(17, 1);
			arrData.insert(17, m_cOneKeyStatus);
			QByteArray arrNew = arrData.left(arrData.length() - 1);
			arrNew.append(getCheckSum(arrNew));
			sendDataToSerial(arrNew);
			verifyBaseStationPosition();
		}
		return;
	}

	//根据记录数据改为查询标定状态指令，直到返回数据中的标定状态与之前一直结束标定
	QByteArray arrData = m_arrLastData;
	arrData[2] = 0x20;
	arrData[17] = 0x08;
	QByteArray arrNew = arrData.left(arrData.length() - 1);
	arrNew.append(getCheckSum(arrNew));
	sendDataToSerial(arrNew);
	m_nOnekeySetIndex++;
	ui.progressBar->setValue(m_nOnekeySetIndex);
}

void PlaceInfoDialog::sendDataToSerial(QByteArray data)
{
	if (data.isEmpty()) return;
	if (m_serialPort.isOpen()) {
		m_serialPort.write(data);
	}
}

void PlaceInfoDialog::verifyBaseStationPosition()
{
	ui.btnRead->setEnabled(true);
	m_timerOnekeyStatus.stop();
	m_bOnekeySetNLINK = false;
	m_nOnekeySetIndex = 0;
	//标定完成后检查标定位置是否符合使用条件
	qDebug() << "一键标定完成检查数据";
	//检查标定基站位置 -8388为无效值
	double xmax = 0;
	double ymax = 0;
	for (int row = 0; row < 4; row++) {
		//检查前6行数值，对应6个基站
		for (int column = 0; column < 2; column++) {
			double value = ui.tableWidget->item(row, column)->text().toDouble();
			if (0 == row && 0 != value) {
				//第一行A0基站必须全为0
				QString error = tr("A0基站标定失败，请重试");
				qWarning() << error;
				QMessageBox::warning(this, tr("提示"), error);
				return;
			}
			if (_InvalidValue_ == value) {
				QString error = QString("A%1基站标定失败，请重试").arg(row);
				qWarning() << error;
				QMessageBox::warning(this, tr("提示"), error);
				return;
			}
			if (0 == column) xmax = qMax(xmax, value);
			if (1 == column) ymax = qMax(ymax, value);
		}
	}
	QString text = QString("基站范围:%1米 X %2米").arg(xmax).arg(ymax);
	qInfo() << text << m_pointPlace;
	ui.labelStationSpace->setText(text);
	//判断场地是否太小或太大
#ifndef _DebugApp_
	//不判断场地大小
	//if (xmax > 52 || ymax > 52) {
	//	QString error = tr("基站范围[%1米X%2米]大于[52米X52米]无法使用请检查后重新标定").arg(xmax).arg(ymax);
	//	qWarning() << error;
	//	QMessageBox::warning(this, tr("提示"), error);
	//	return;
	//}
	//if (xmax < 5 || ymax < 5) {
	//	QString error = tr("基站范围[%1米X%2米]小于[5米X5米]无法使用请检查后重新标定").arg(xmax).arg(ymax);
	//	qWarning() << error;
	//	QMessageBox::warning(this, tr("提示"), error);
	//	return;
	//}
	//double currentX = m_pointPlace.x() / 100.0;
	//if (xmax < currentX) {
	//	QString error = QString("X轴方向长度%1米比项目设定场地长度%2米小，基站无法满足飞行范围").arg(xmax).arg(currentX);
	//	qWarning() << error;
	//	QMessageBox::warning(this, tr("提示"), error);
	//	return;
	//}
	//double currentY = m_pointPlace.y() / 100.0;
	//if (ymax < currentY) {
	//	QString error = QString("Y轴方向长度%1米比项目设定场地长度%2米小，基站无法满足飞行范围").arg(xmax).arg(currentY);
	//	qWarning() << error;
	//	QMessageBox::warning(this, tr("提示"), error);
	//	return;
	//}
#endif
	m_stationStatus = 1;
	ui.progressBar->setValue(ui.progressBar->maximum());
	qInfo() << "一键标定完成并且位置可用";
	QMessageBox::information(this, tr("提示"), tr("标定成功"));
	return;
}

void PlaceInfoDialog::clearStatus()
{
	if (m_timerOnekeyStatus.isActive()) {
		m_timerOnekeyStatus.stop();
	}
	ui.btnRead->setEnabled(true);
	ui.progressBar->setValue(0);
	m_nOnekeySetIndex = 0;
	ui.labelStationSpace->setText("串口已断开");
}

