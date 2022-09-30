#include "devicemanage.h"
#include "qmath.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include "definesetting.h"
#include "messagelistdialog.h"
#include "devicedebug.h"
#include "define3d.h"

#define _ItemHeight_ 60
DeviceManage::DeviceManage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	
	m_p3dTcpSocket = nullptr;
	m_p3dTcpServer = new QTcpServer(this);
	m_p3dTcpServer->listen(QHostAddress::Any, _TcpPort_);
	connect(m_p3dTcpServer, SIGNAL(newConnection()), this, SLOT(on3dNewConnection()));

	//设置设备列表
	ui.listWidget->installEventFilter(this);
	connect(ui.listWidget, &QListWidget::currentItemChanged, [this](QListWidgetItem* current, QListWidgetItem* previous) {
		qDebug() << "list currentItemChanged" << current; 
		QString name;
		QString previousname;
		if (current) {
			DeviceControl* pControl = dynamic_cast<DeviceControl*>(ui.listWidget->itemWidget(current));
			if (pControl) name = pControl->getName();
		}
		if (previous) {
			DeviceControl* pControl = dynamic_cast<DeviceControl*>(ui.listWidget->itemWidget(previous));
			if (pControl) previousname = pControl->getName();
		}
		emit currentDeviceNameChanged(name, previousname);
		});

	//添加右键菜单
	m_pMenu = new QMenu(this);
	QAction* pActionResetName = new QAction(tr("重命名"), this);
	QAction* pActionResetIP = new QAction(tr("修改IP"), this);
	QAction* pActionDicconnect = new QAction(tr("连接/断开"), this);
	QAction* pFlyTo = new QAction(tr("起飞"), this);
	QAction* pLand = new QAction(tr("降落"), this);
	QAction* pStop = new QAction(tr("急停"), this);
	QAction* pDebug = new QAction(tr("调试"), this);
	m_pMenu->addAction(pActionResetName);
	m_pMenu->addAction(pActionResetIP);
	m_pMenu->addAction(pActionDicconnect);
	m_pMenu->addSeparator();
	m_pMenu->addAction(pFlyTo);
	m_pMenu->addAction(pLand);
	m_pMenu->addAction(pStop);
	m_pMenu->addSeparator();
	m_pMenu->addAction(pDebug);
	//菜单响应处理
	connect(pActionResetName, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		QString qstrOldName = pControl->getName();
		QString qstrNewName = QInputDialog::getText(this, tr("重命名"), tr("请输入新名称"), QLineEdit::Normal, qstrOldName);
		if (qstrNewName.isEmpty()) return;
		if (qstrOldName == qstrNewName) return;
		if (!isRepetitionDevice(qstrNewName, "",0,0).isEmpty()) {
			QMessageBox::warning(this, tr("提示"), tr("无法修改，名称重复"));
			return;
		}
		pControl->setName(qstrNewName);
		emit deviceRenameFinished(qstrNewName, qstrOldName);

		if (m_p3dTcpSocket) {
			QJsonObject obj3dmsg;
			obj3dmsg.insert(_Ver_, _VerNum_);
			obj3dmsg.insert(_Tag_, _TabName_);
			obj3dmsg.insert(_ID_, _3dDeviceRename);
			obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
			QJsonObject data;
			data.insert("oldName", qstrOldName);
			data.insert("newName", qstrNewName);
			obj3dmsg.insert(_Data_, data);
			sendMessageTo3D(obj3dmsg);
		}
		});
	connect(pActionResetIP, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		QString qstrNewIP = QInputDialog::getText(this, tr("IP"), tr("请输入设备IP"), QLineEdit::Normal, pControl->getIP());
		if (qstrNewIP.isEmpty()) return;
		if (pControl->getIP() == qstrNewIP) return;
		if (!isRepetitionDevice("", qstrNewIP, 0, 0).isEmpty()) {
			QMessageBox::warning(this, tr("提示"), tr("无法修改，设备地址重复"));
			return;
		}
		pControl->setIp(qstrNewIP);
		emit deviceResetIp(pControl->getName(), qstrNewIP);
		});
	connect(pActionDicconnect, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		if (pControl->isConnectDevice()) {
			pControl->disconnectDevice();
		}
		else {
			pControl->connectDevice();
		}
		});
	connect(pFlyTo, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		int res = pControl->Fun_MAV_CMD_NAV_TAKEOFF_LOCAL(0, 0, 0, 0, 0, 0, _TakeoffLocalHeight_);
		if (res == _DeviceStatus::DeviceDataSucceed) return;
		_ShowErrorMessage(pControl->getName() + tr("起飞") + Utility::waypointMessgeFromStatus(res));
		});
	connect(pLand, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		int res = pControl->Fun_MAV_CMD_NAV_LAND_LOCAL(0, 0, 0, 0, 0, 0, 0);
		if (res == _DeviceStatus::DeviceDataSucceed) return;
		_ShowErrorMessage(pControl->getName() + tr("降落") + Utility::waypointMessgeFromStatus(res));
		});
	connect(pStop, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		int res = pControl->Fun_MAV_QUICK_STOP();
		if (res == _DeviceStatus::DeviceDataSucceed) return;
		_ShowErrorMessage(pControl->getName() + tr("急停") + Utility::waypointMessgeFromStatus(res));;
		});
	connect(pDebug, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		DeviceDebug* pDebug = pControl->getDeviceDebug();
		if (!pDebug) return;
		pDebug->setName(pControl->getName());
		pDebug->setIp(pControl->getIP());
		pDebug->show();
		});
	//addDevice("测试名称1", "", 0,0);
	//addDevice("测试名称2", "", 0, 0);
	connect(ui.btnAddDevice, &QAbstractButton::clicked, [this]() {
		AddDeviceDialog dialog(getNewDefaultName(), this);
		if (QDialog::Accepted != dialog.exec())return;
		QString qstrError = addDevice(dialog.getName(), dialog.getIP(), dialog.getX(), dialog.getY());
		if (!qstrError.isEmpty()) {
			QMessageBox::warning(this, tr("错误"), qstrError);
		}
		});
	connect(ui.btnRemoveDevice, &QAbstractButton::clicked, [this]() { removeDevice(); });

	connect(ui.btnFlyTakeoff, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceTakeoffLocal); });
	connect(ui.btnFlyLand, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceLandLocal); });
	connect(ui.btnFlyStop, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceQuickStop); });

	ui.checkBoxAutoLand->setVisible(false);
	ui.checkBoxMagnetismStatus->setVisible(false);
}

DeviceManage::~DeviceManage()
{
	if (m_p3dTcpSocket) {
		m_p3dTcpSocket->close();
		delete m_p3dTcpSocket;
		m_p3dTcpSocket = nullptr;
	}
	if (m_p3dTcpServer) {
		m_p3dTcpServer->close();
		delete m_p3dTcpServer;
		m_p3dTcpServer = nullptr;
	}
}

QString DeviceManage::addDevice(QString qstrName, QString ip, float x, float y)
{
	if (qstrName.isEmpty()) return tr("设备名称不能为空");
	//判断设备是否重复
	QString temp = isRepetitionDevice(qstrName, ip, x, y, true);
	if (!temp.isEmpty())  return temp;

	QListWidgetItem* item = new QListWidgetItem();
	item->setSizeHint(QSize(0, _ItemHeight_));
	ui.listWidget->addItem(item);
	//此处会耗时
	DeviceControl* pControl = new DeviceControl(qstrName, x, y, ip);
	connect(pControl, &DeviceControl::sigWaypointProcess, this, &DeviceManage::sigWaypointProcess);
	connect(pControl, &DeviceControl::sigConrolFinished, this, &DeviceManage::onDeviceConrolFinished);
	ui.listWidget->setItemWidget(item, pControl);
	ui.listWidget->setCurrentItem(item);
	emit deviceAddFinished(qstrName, ip, x, y);
	if (m_p3dTcpSocket) {
		QJsonObject obj3dmsg;
		obj3dmsg.insert(_Ver_, _VerNum_);
		obj3dmsg.insert(_Tag_, _TabName_);
		obj3dmsg.insert(_ID_, _3dDeviceAdd);
		obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		QJsonObject data;
		data.insert("name", qstrName);
		data.insert("x", x);
		data.insert("y", y);
		obj3dmsg.insert(_Data_, data);
		sendMessageTo3D(obj3dmsg);
	}
	return "";
}

void DeviceManage::removeDevice()
{
	//删除选中项目，如果没有选中则删除最后一个
	int count = ui.listWidget->count();
	if (0 >= count) return;
	QListWidgetItem* item = ui.listWidget->currentItem();
	if (!item) item = ui.listWidget->item(count - 1);
	DeviceControl* m_pControl = dynamic_cast<DeviceControl*>(ui.listWidget->itemWidget(item));
	if (!m_pControl) return;
	QString name = m_pControl->getName();
	ui.listWidget->takeItem(ui.listWidget->currentRow());
	emit deviceRemoveFinished(name);

	if (m_p3dTcpSocket) {
		QJsonObject obj3dmsg;
		obj3dmsg.insert(_Ver_, _VerNum_);
		obj3dmsg.insert(_Tag_, _TabName_);
		obj3dmsg.insert(_ID_, _3dDeviceRemove);
		obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		QJsonObject data;
		data.insert("name", name);
		obj3dmsg.insert(_Data_, data);
		sendMessageTo3D(obj3dmsg);
	}
}

void DeviceManage::clearDevice()
{
	if (m_p3dTcpSocket) {
		for (int i = 0; i < ui.listWidget->count(); i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			QJsonObject obj3dmsg;
			obj3dmsg.insert(_Ver_, _VerNum_);
			obj3dmsg.insert(_Tag_, _TabName_);
			obj3dmsg.insert(_ID_, _3dDeviceRemove);
			obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
			obj3dmsg.insert("name", pDevice->getName());
			sendMessageTo3D(obj3dmsg);
		}
	}
	ui.listWidget->clear();
}

QString DeviceManage::getCurrentDeviceName()
{
	DeviceControl* pControl = getCurrentDevice();
	if (!pControl) return "";
	return pControl->getName();
}

bool DeviceManage::setCurrentDevice(QString qstrName)
{
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		if(pDevice->getName() != qstrName) continue;
		ui.listWidget->setCurrentItem(pItem);
		qDebug() << "选中设备" << qstrName;
		return true;
	}
	return false;
}

QStringList DeviceManage::getDeviceNameList()
{
	QStringList list;
	if (0 >= ui.listWidget->count()) return list;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		list.append(pDevice->getName());
	}
	return list;
}

//获取可用的新设备名称
QString DeviceManage::getNewDefaultName()
{
	int index = 0;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		QString name = pDevice->getName();
		QStringList list = name.split(_DeviceNamePrefix_);
		if (list.count() < 2) continue;
		int n = list.at(1).toInt();
		index = qMax(index, n);
	}
	return QString("%1%2").arg(_DeviceNamePrefix_).arg(index + 1);
}

//设备名称是否重复
QString DeviceManage::isRepetitionDevice(QString qstrName, QString ip, float x, float y, bool location)
{
	location = false;
	if (0 >= ui.listWidget->count()) return false;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		if (!qstrName.isEmpty()) {
			if (qstrName == pDevice->getName()) return tr("设备名称重复");
		}
		if (!ip.isEmpty()) {
			if (ip == pDevice->getIP()) return tr("设备IP重复");
		}
		if (location) {
			if (x == pDevice->getX() && y == pDevice->getY()) return tr("设备初始位置重复");
		}
	}
	return "";
}

void DeviceManage::allDeviceControl(_AllDeviceCommand comand)
{
	if (DeviceManage::_DeviceTakeoffLocal == comand) {
		//起飞前增加倒计时
		QLabel label(dynamic_cast<QWidget*>(parent()));
		//设置窗体的背景色,这里的百分比就是透明度
		label.setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);color:#FFFFFF;"));
		label.setGeometry(dynamic_cast<QWidget*>(parent())->rect()); //获取父窗体的几何形状设置当前窗口
		label.show();
		label.setAlignment(Qt::AlignCenter);
		QFont font = label.font();
		font.setPointSize(font.pointSize() * 6);
		label.setFont(font);
		label.setText("3");
		QTimer timer;
		timer.start(1000);
		int index = 1;
		connect(&timer, &QTimer::timeout, [&, this]() {
			index++;
			label.setText(QString::number(label.text().toInt() - 1));
			if (index > 3) timer.stop();
			});
		while (timer.isActive()) {
			QApplication::processEvents();
		}
	}
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		QString qstrName = pDevice->getName();
		int res = DeviceDataSucceed;
		QString qstrText;
		switch (comand)
		{
		case DeviceManage::_DeviceTakeoffLocal:
			res = pDevice->Fun_MAV_CMD_NAV_TAKEOFF_LOCAL(0, 0, 0, 0, 0, 0, _TakeoffLocalHeight_, false);
			qstrText = tr("起飞");
			break;
		case DeviceManage::_DeviceLandLocal:
			res = pDevice->Fun_MAV_CMD_NAV_LAND_LOCAL(0, 0, 0, 0, 0, 0, 0, false);
			qstrText = tr("降落");
			break;
		case DeviceManage::_DeviceQuickStop:
			res = pDevice->Fun_MAV_QUICK_STOP(false);
			qstrText = tr("急停");
			break;
		case DeviceManage::_DeviceSetout:
			res = pDevice->Fun_MAV_CMD_DO_SET_MODE(3, false);
			qstrText = tr("准备起飞");
			break;
		default:
			break;
		}
		if (_DeviceStatus::DeviceDataSucceed != res) {
			_ShowErrorMessage(qstrName+qstrText+tr("出错:")+ Utility::waypointMessgeFromStatus(res));
		}
	}
	if (DeviceManage::_DeviceTakeoffLocal == comand) emit sigTakeoffFinished(true);
	else emit sigTakeoffFinished(false);
}

void DeviceManage::allDeviceCalibration(_CalibrationEnum c)
{

}

QString DeviceManage::sendWaypoint(QString name, QVector<NavWayPointData> data, bool upload)
{
	if (name.isEmpty()) return tr("设备名称错误");
	if (0 == data.count()) return tr("舞步数据为空");
	
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		if (name != pDevice->getName()) continue;
		//舞步前添加起始位置
		NavWayPointData startLocation;
		startLocation.x = pDevice->getX() * 1000;
		startLocation.y = pDevice->getX() * 1000;
		startLocation.commandID = 31004;
		data.prepend(startLocation);

		if (m_p3dTcpSocket) {
			QJsonObject obj3dmsg;
			obj3dmsg.insert(_Ver_, _VerNum_);
			obj3dmsg.insert(_Tag_, _TabName_);
			obj3dmsg.insert(_ID_, _3dDeviceWaypoint);
			obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
			QJsonObject objDevice;
			objDevice.insert("name", name);
			QJsonArray arrWaypoint;
			for (int i = 0; i < data.count(); i++) {
				QList<QVariant> value;
				value << data.at(i).param1 << data.at(i).param2 << data.at(i).param3 << data.at(i).param4 
					<< data.at(i).x << data.at(i).y << data.at(i).z 
					<< data.at(i).commandID;
				arrWaypoint.append(QJsonArray::fromVariantList(value));
			}
			objDevice.insert("list", arrWaypoint);
			QJsonArray arrData;
			arrData.append(objDevice);
			obj3dmsg.insert(_Data_, arrData);
			sendMessageTo3D(obj3dmsg);
		}
		if (upload) {
			int status = pDevice->DeviceMavWaypointStart(data);
			if (_DeviceStatus::DeviceDataSucceed != status) return Utility::waypointMessgeFromStatus(status);
		}
	}
	return "";
}

void DeviceManage::setUpdateWaypointTime(int second)
{
	QJsonObject obj3dmsg;
	obj3dmsg.insert(_Ver_, _VerNum_);
	obj3dmsg.insert(_Tag_, _TabName_);
	obj3dmsg.insert(_ID_, _3dDeviceTime);
	obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	obj3dmsg.insert(_Data_, second);
	sendMessageTo3D(obj3dmsg);
}

void DeviceManage::setCurrentPlayeState(qint8 state)
{
	return;
	QJsonObject obj3dmsg;
	obj3dmsg.insert(_Ver_, _VerNum_);
	obj3dmsg.insert(_Tag_, _TabName_);
	obj3dmsg.insert(_ID_, _3dDeviceAction);
	obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	obj3dmsg.insert(_Data_, state);
	sendMessageTo3D(obj3dmsg);
}

void DeviceManage::setCurrentMusicPath(QString filePath)
{
	if (!QFile::exists(filePath)) return;
	QJsonObject obj3dmsg;
	obj3dmsg.insert(_Ver_, _VerNum_);
	obj3dmsg.insert(_Tag_, _TabName_);
	obj3dmsg.insert(_ID_, _3dDeviceMusicPath);
	obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	obj3dmsg.insert(_Data_, filePath);
	sendMessageTo3D(obj3dmsg);
}

bool DeviceManage::eventFilter(QObject* watched, QEvent* event)
{
	if (!isEnabled()) return false;
	if (ui.listWidget == watched) {
		//设备列表菜单
		if (QEvent::ContextMenu != event->type()) return false;
		if (!ui.listWidget->itemAt(ui.listWidget->mapFromGlobal(QCursor::pos()))) return false;
		m_pMenu->exec(QCursor::pos());
	} 
	return false;
}

void DeviceManage::on3dNewConnection()
{
	if (!m_p3dTcpServer) return;
	if (m_p3dTcpSocket) {
		m_p3dTcpSocket->disconnect();
		m_p3dTcpSocket->close();
	}
	m_p3dTcpSocket = m_p3dTcpServer->nextPendingConnection();
	connect(m_p3dTcpSocket, &QTcpSocket::readyRead, [this]() {
		QByteArray arrData = m_p3dTcpSocket->readAll();
		qDebug() << "收到3D消息:" << arrData;
		});
	connect(m_p3dTcpSocket, &QTcpSocket::disconnected, [this]() {
		emit sig3DDialogStatus(false);
		m_p3dTcpSocket = nullptr;
		});
	//发送无人机设备列表
	QJsonObject obj3dmsg;
	obj3dmsg.insert(_Ver_, _VerNum_);
	obj3dmsg.insert(_Tag_, _TabName_);
	obj3dmsg.insert(_ID_, _3dDeviceList);
	obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	QJsonArray jsonArr;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		QJsonObject device;
		device.insert("name", pDevice->getName());
		device.insert("x", pDevice->getX());
		device.insert("y", pDevice->getY());
		jsonArr.append(device);
	}
	obj3dmsg.insert(_Data_, jsonArr);
	sendMessageTo3D(obj3dmsg);
	emit sig3DDialogStatus(true);
}

DeviceControl* DeviceManage::getCurrentDevice()
{
	QListWidgetItem* item = ui.listWidget->currentItem();
	if (!item) return nullptr;
	QWidget* pWidget = ui.listWidget->itemWidget(item);
	if (!pWidget) return nullptr;
	DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
	return pDevice;
}

void DeviceManage::sendMessageTo3D(QJsonObject json3d)
{
	if (nullptr == m_p3dTcpSocket) return;
	QJsonDocument document(json3d);
	QByteArray msg3d = document.toJson(QJsonDocument::Compact);
	qDebug() << "3dmessage" << json3d;
	m_p3dTcpSocket->write(msg3d);
}

void DeviceManage::onDeviceConrolFinished(QString text, int res, QString explain)
{
	if (DeviceDataSucceed == res) return;
	DeviceControl* pControl = dynamic_cast<DeviceControl*>(sender());
	if (!pControl) return;
	text.prepend(pControl->getName());
	_ShowErrorMessage(text + tr("错误:") + Utility::waypointMessgeFromStatus(res));
}