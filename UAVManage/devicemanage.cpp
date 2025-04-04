#include "devicemanage.h"
#include "qmath.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include "definesetting.h"
#include "messagelistdialog.h"
#include "devicedebug.h"
#include "define3d.h"
#include "calibrationdialog.h"
#include "deviceserial.h"
#include "firmwaredialog.h"
#include "placeinfodialog.h"
#include "tinyxml2/tinyxml2.h"

#define _ItemHeight_ 76
DeviceManage::DeviceManage(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_pointSpace.setX(0);
	m_pointSpace.setY(0);
	m_b3DFinished = false;
	m_pFirmwareDialog = nullptr;
	m_p3dTcpSocket = nullptr;
	m_bControlIng = false;
	m_p3dTcpServer = new QTcpServer(this);
	ui.widgetPreflightCheck->setVisible(false);
	connect(m_p3dTcpServer, SIGNAL(newConnection()), this, SLOT(on3dNewConnection()));
	//设置设备列表
	ui.listWidget->installEventFilter(this);
	ui.listWidget->setSpacing(4);
	connect(ui.listWidget, &QListWidget::currentItemChanged, [this](QListWidgetItem* current, QListWidgetItem* previous) {
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
	m_bDebug = false;
	
	//初始化粘贴菜单
	m_pMenuPaste = new QMenu(this);
	m_pActionPaste = new QAction(tr("粘贴"), this);
	m_pMenuPaste->addAction(m_pActionPaste);
	connect(m_pActionPaste, &QAction::triggered, this, &DeviceManage::onActionPaste);

	m_pMenu = new QMenu(this);
	m_pMenu->setWindowFlags(m_pMenu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	m_pMenu->setAttribute(Qt::WA_TranslucentBackground);
	QAction* pActionParam = new QAction(tr("修改"), this);
	QAction* pActionDicconnect = new QAction(tr("连接/断开"), this);
	QAction* pActionCopy = new QAction(tr("复制"), this);
	QAction* pFlyTo = new QAction(tr("起飞"), this);
	QAction* pLand = new QAction(tr("降落"), this);
	QAction* pActionMagnetism = new QAction(tr("磁罗盘校准"), this);
	QAction* pActionAccelerometer = new QAction(tr("加计校准"), this);
	m_pActionMagnetismOpen = new QAction(tr("磁罗盘开启"), this);
	m_pActionMagnetismClose = new QAction(tr("磁罗盘关闭"), this);
	m_pActionGyro = new QAction(tr("陀螺校准"), this);
	m_pActionBaro = new QAction(tr("电调校准"), this);
	m_pActionDebug = new QAction(tr("调试"), this);
	QAction* pActionLocate = new QAction(tr("手动定位"), this);
	m_pMenu->addAction(pActionParam);
	m_pMenu->addAction(pActionDicconnect);
	m_pMenu->addSeparator();
	m_pMenu->addAction(pActionCopy);
	m_pMenu->addSeparator();
	m_pMenu->addAction(pFlyTo);
	m_pMenu->addAction(pLand);
	m_pMenu->addSeparator();
	m_pMenu->addAction(pActionMagnetism);
	m_pMenu->addAction(pActionAccelerometer);
	m_pMenu->addSeparator();
	//m_pMenu->addAction(pActionLocate);
	m_pActionGyro->setProperty("Calibration", _Gyro);
	pActionMagnetism->setProperty("Calibration", _Magnetometer);
	pActionAccelerometer->setProperty("Calibration", _Accelerometer);
	m_pActionBaro->setProperty("Calibration", _Baro);
	connect(pActionLocate, &QAction::triggered, this, &DeviceManage::onActionLocateDevice);
	connect(&m_timerLocate, &QTimer::timeout, this, &DeviceManage::onActionLocateDevice);
	connect(m_pActionGyro, &QAction::triggered, this, &DeviceManage::deviceCalibration);
	connect(pActionMagnetism, &QAction::triggered, this, &DeviceManage::deviceCalibration);
	connect(pActionAccelerometer, &QAction::triggered, this, &DeviceManage::deviceCalibration);
	connect(m_pActionBaro, &QAction::triggered, this, &DeviceManage::deviceCalibration);
	connect(m_pActionMagnetismOpen, &QAction::triggered, [this]() {
		DeviceControl* pDevice = getCurrentDevice();
		if (nullptr == pDevice) return;
		int res = pDevice->Fun_MAV_CALIBRATION(0, 0, 1, 0, 0, 0, 0);
		if (DeviceDataSucceed != res) {
			_ShowErrorMessage(pDevice->getName() + tr("磁罗盘开启") + Utility::waypointMessgeFromStatus(_DeviceCalibration, res));
			return;
		}
		_ShowInfoMessage(pDevice->getName() + tr("磁罗盘开启成功"));
		});
	connect(m_pActionMagnetismClose, &QAction::triggered, [this]() {
		DeviceControl* pDevice = getCurrentDevice();
		if (nullptr == pDevice) return;
		int res = pDevice->Fun_MAV_CALIBRATION(0, 0, 0, 0, 0, 0, 0);
		if (DeviceDataSucceed != res) {
			_ShowErrorMessage(pDevice->getName() + tr("磁罗盘关闭") + Utility::waypointMessgeFromStatus(_DeviceCalibration, res));
			return;
		}
		_ShowInfoMessage(pDevice->getName() + tr("磁罗盘关闭成功"));
		});
	connect(pActionCopy, &QAction::triggered, [this]() {
		m_qstrCopyName = getCurrentDeviceName();
		qInfo() << "已复制设备" << m_qstrCopyName;
		});
	//菜单响应处理
	connect(pActionParam, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		QString qstrName = pControl->getName();
		AddDeviceDialog dialog(qstrName, m_pointSpace.x(), m_pointSpace.y(), this);
		dialog.setParam(qstrName, pControl->getIP(), pControl->getX(), pControl->getY());
		if (QDialog::Accepted != dialog.exec())return;
		QString qstrNewName = dialog.getName();
		QString qstrNewIP = dialog.getIP();
		long nNewX = dialog.getX();
		long nNewY = dialog.getY();
		if (qstrNewName != qstrName) {
			//修改设备名称
			if (!isRepetitionDevice(qstrNewName, "", 0, 0, "100").isEmpty()) {
				QMessageBox::warning(this, tr("提示"), tr("设备名称重复"));
				return;
			}
			qDebug() << "修改设备名称" << qstrName << qstrNewName;
			pControl->setName(qstrNewName);
			if (m_qstrCopyName == qstrName) m_qstrCopyName = qstrNewName;
			emit deviceRenameFinished(qstrNewName, qstrName);
			emit currentDeviceNameChanged(qstrNewName, qstrName);
		}
		if (qstrNewIP != pControl->getIP()) {
			//修改设备IP
			QString text = isRepetitionDevice("", qstrNewIP, 0, 0, "010");
			if (!isRepetitionDevice("", qstrNewIP, 0, 0, "010").isEmpty()) {
				QMessageBox::warning(this, tr("提示"), tr("设备地址重复"));
				return;
			}
			qDebug() << "修改设备IP" << qstrNewName << qstrNewIP;
			pControl->setIp(qstrNewIP);
			emit deviceResetIp(qstrNewName, qstrNewIP);
		}
		if (nNewX != pControl->getX() || nNewY != pControl->getY()) {
			//修改设备初始位置
			if (!isRepetitionDevice("", "", nNewX, nNewY, "001").isEmpty()) {
				QMessageBox::warning(this, tr("提示"), tr("初始位置重复"));
				return;
			}
			qDebug() << "修改初始位置" << qstrNewName << nNewX << nNewY;
			pControl->setStartLocation(nNewX, nNewY);
			emit deviceResetLocation(qstrNewName, nNewX, nNewY);
		}
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
		_ShowErrorMessage(pControl->getName() + tr("起飞") + Utility::waypointMessgeFromStatus(_DeviceTakeoffLocal, res));
		});
	connect(pLand, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		int res = pControl->Fun_MAV_CMD_NAV_LAND_LOCAL(0, 0, 0, 0, 0, 0, 0);
		if (res == _DeviceStatus::DeviceDataSucceed) return;
		_ShowErrorMessage(pControl->getName() + tr("降落") + Utility::waypointMessgeFromStatus(_DeviceLandLocal, res));
		});
	connect(m_pActionDebug, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		DeviceDebug* pDebug = pControl->getDeviceDebug();
		if (!pDebug) return;
		pDebug->setName(pControl->getName());
		pDebug->setIp(pControl->getIP());
		pDebug->show();
		});
	connect(ui.btnAddDevice, &QAbstractButton::clicked, [this]() {
		AddDeviceDialog dialog(getNewDefaultName(), m_pointSpace.x(), m_pointSpace.y(), this);
		QPoint point = getNewDevicePoint();
		dialog.setX(point.x());
		dialog.setY(point.y());
		if (QDialog::Accepted != dialog.exec())return;
		QString qstrError = addDevice(dialog.getName(), dialog.getIP(), dialog.getX(), dialog.getY());
		if (!qstrError.isEmpty()) {
			QMessageBox::warning(this, tr("错误"), qstrError);
		}
		});
	connect(ui.btnSelectAll, &QAbstractButton::clicked, [this]() {
		for (int i = 0; i < ui.listWidget->count(); i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			pDevice->setChcekStatus(true);
		}
		});
	connect(ui.btnReverse, &QAbstractButton::clicked, [this]() {
		for (int i = 0; i < ui.listWidget->count(); i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			pDevice->setChcekStatus(!pDevice->isCheckDevice());
		}
		});
	connect(ui.btnCheckAction, &QAbstractButton::clicked, this, &DeviceManage::sigPrepareWidget);
	connect(ui.btnClose, &QAbstractButton::clicked, this, &DeviceManage::sigPrepareWidget);
	connect(ui.btnWaypointCheck, &QAbstractButton::clicked, [this]() { waypointComposeAndUpload(m_qstrCurrentProjectFile, false); });
	connect(ui.btn3DFly, &QAbstractButton::clicked, [this]() { emit sigStart3D(); });
	connect(ui.btnWaypointUpload, &QAbstractButton::clicked, [this]() { waypointComposeAndUpload(m_qstrCurrentProjectFile, true); });
	connect(ui.btnConsistent, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceTimeSync); });
	connect(ui.btnDevicePrepare, &QAbstractButton::clicked, [this]() { allDeviceControl(_DevicePrepare); });
	connect(ui.btnFlyTakeoff, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceTakeoffLocal); });
	connect(ui.btnFlyLand, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceLandLocal); });
	connect(ui.btnFlyStop, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceQuickStop); });
	connect(ui.btnQueue, &QAbstractButton::clicked, [this]() {
		QMessageBox::information(this, "提示", "此功能正在功能开发中......");
		return;
		allDeviceControl(_DeviceQueue); });
	connect(ui.btnRegain, &QAbstractButton::clicked, [this]() {
		QMessageBox::information(this, "提示", "此功能正在功能开发中......");
		return;
		allDeviceControl(_DeviceRegain); });
	connect(&m_timerUpdateStatus, &QTimer::timeout, this, &DeviceManage::onUpdateStatusTo3D);
	connect(&m_timerMessage3D, &QTimer::timeout, this, &DeviceManage::onTimeout3DMessage);
	connect(&m_timerSync, &QTimer::timeout, this, &DeviceManage::onTimeoutSync);
	connect(ui.btnBaseStation, &QAbstractButton::clicked, [this]() {
#ifdef _UseUWBData_
		m_pUWBStation->closeSeial();
#endif
		PlaceInfoDialog info(getSpaceSize(), this);
		info.exec();
#ifdef _UseUWBData_
		m_pUWBStation->openSerial();
#endif
		if (info.isUpdateStation()) ui.labelStationStatus->setText("<font color=#FF0000>基站标定未完成</font>");
		if (false == info.isValidStation()) return;
		QMap<QString, QPoint> map = info.getStationAddress();
		setStationAddress(map);
		ui.labelStationStatus->setText("<font color=#467FC1>基站标定已完成</font>");
		});
	//设备IP地址信息
	m_pDeviceNetwork = new DeviceSerial(this);
	ui.btnSerial->setVisible(m_pDeviceNetwork->isSerialEnabled());
	connect(m_pDeviceNetwork, &DeviceSerial::sigDeviceEnabled, [this](bool enabled) { ui.btnSerial->setVisible(enabled); });
	connect(ui.btnSerial, &QAbstractButton::clicked, [this]() { m_pDeviceNetwork->exec(); });

#ifdef _UseUWBData_
	m_pUWBStation = new UWBStationData(this);
	connect(m_pUWBStation, &UWBStationData::sigConnectStatus, this, &DeviceManage::onUWBConnectStatus);
	connect(m_pUWBStation, &UWBStationData::sigReceiveData, this, &DeviceManage::onUWBReceiveData);
#else 
	ui.labelUWBStatus->setVisible(false);
#endif
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
	if (m_pDeviceNetwork) {
		m_pDeviceNetwork->close();
		m_pDeviceNetwork->deleteLater();
		m_pDeviceNetwork = nullptr;
	}
}

bool DeviceManage::start3DTcp()
{
	return m_p3dTcpServer->listen(QHostAddress::Any, _TcpPort_);
}

void DeviceManage::setSpaceSize(unsigned int x, unsigned int y)
{
	m_pointSpace.setX(x);
	m_pointSpace.setY(y);
	qDebug() << "设定场地大小" << x << y;
}

QPoint DeviceManage::getSpaceSize()
{
	return m_pointSpace;
}

void DeviceManage::setStationAddress(QMap<QString, QPoint> station)
{
	m_stationMap = station;
}

QString DeviceManage::addDevice(QString qstrName, QString ip, long x, long y, bool bUpdateBlockly)
{
	int count = ui.listWidget->count();
	if (count >= _MaxDevice_) {
		return tr("无法添加新无人机，已超出最大控制数量");
	}
	if (qstrName.isEmpty()) return tr("设备名称不能为空");
#ifdef _UseUWBData_
	if (ip.isEmpty()) {
		int nTag = ip.toInt();
		QList<int> listCount;
		for (int i = 1; i <= 40; i++) {
			listCount.append(i);
		}
		for (int i = 0; i < ui.listWidget->count(); i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			listCount.removeOne(pDevice->getIP().toInt());
		}
		if (0 == listCount.count()) {
			return "超过无人机最大标签数量";
		}
		int temp = listCount.first();
		ip = QString::number(temp);
	}
#endif

	//判断设备是否重复
	QString temp = isRepetitionDevice(qstrName, ip, x, y, "111");
	if (!temp.isEmpty())  return temp;

	QListWidgetItem* item = new QListWidgetItem();
	item->setSizeHint(QSize(0, _ItemHeight_));
	ui.listWidget->addItem(item);
	//此处会耗时
	DeviceControl* pControl = new DeviceControl(qstrName, x, y, ip);
	connect(pControl, &DeviceControl::sigWaypointFinished, this, &DeviceManage::onWaypointFinished);
	connect(pControl, &DeviceControl::sigConrolFinished, this, &DeviceManage::onDeviceConrolFinished);
	connect(pControl, &DeviceControl::sigRemoveDevice, this, &DeviceManage::onRemoveDevice);
#ifdef _UseUWBData_
	connect(pControl, &DeviceControl::sigSendDataToUWB, this, &DeviceManage::onSendDataToUWB);
#endif
	//需要先发送添加设备信息，用于创建默认blockly布局，当ui.listWidget->setCurrentItem触发设备切换时可以显示有布局的WEB界面
	ui.listWidget->setItemWidget(item, pControl);
	if (bUpdateBlockly) {
		emit deviceAddFinished(qstrName, ip, x, y);
		ui.listWidget->setCurrentItem(item);
	}
	return "";
}

void DeviceManage::clearDevice()
{
	ui.listWidget->clear();
	m_qstrCopyName.clear();
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
		if (pDevice->getName() != qstrName) continue;
		qDebug() << "项目中选中的设备" << qstrName;
		ui.listWidget->setCurrentItem(pItem);
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

QPoint DeviceManage::getNewDevicePoint()
{
	long maxx = 100;
	long maxy = 0;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		maxx = qMax(maxx, pDevice->getX());
		maxy = qMax(maxy, pDevice->getY());
		if (maxx >= pDevice->getX()) {
			maxy = pDevice->getY();
		}
	}
	//设备间隔100厘米
	//以水平Y轴方向顺序排放
	int x = maxx;
	int y = maxy + 100;
	if (y > (m_pointSpace.y() - 100)) {
		y = 100;
		x = maxx + 100;
		if (x >= (m_pointSpace.x() - 100)) x = 100;
	}
	QPoint point(x, y);
	return point;
}

//设备名称是否重复
QString DeviceManage::isRepetitionDevice(QString qstrName, QString ip, long x, long y, QString type)
{
	//location = false;
	if (0 >= ui.listWidget->count()) return false;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		QString qstrRep = type;
		while (qstrRep.length() < 3) {
			qstrRep.prepend("0");
		}
		if (!qstrName.isEmpty() && '0' != qstrRep.at(0)) {
			if (qstrName == pDevice->getName()) return tr("设备名称重复");
		}
		if (!ip.isEmpty() && '0' != qstrRep.at(1)) {
			if (ip == pDevice->getIP()) return tr("设备地址重复");
		}
		if ('0' != qstrRep.at(2)) {
			if (x == pDevice->getX() && y == pDevice->getY()) return tr("设备初始位置重复");
		}
	}
	return "";
}

void DeviceManage::allDeviceControl(_AllDeviceCommand comand)
{
	if (m_bControlIng) {
		//避免反复控制，造成逻辑错误
		_ShowInfoMessage("上一次操控未结束，请等待");
		return;
	}
	if (_DeviceTimeSync == comand) {
		if (m_timerSync.isActive()) {
			QMessageBox::information(this, tr("提示"), tr("定装授时正在进行中"));
			return;
		}
	}
	m_bControlIng = true;
	QString text = "无人机控制";
	switch (comand) {
	case _DeviceTakeoffLocal: text.append("起飞"); break;
	case _DeviceLandLocal: text.append("降落"); break;
	case _DeviceQuickStop: text.append("急停"); break;
	case _DeviceTimeSync: text.append("定桩授时"); break;
	case _DevicePrepare: text.append("准备起飞"); break;
	case _DeviceQueue: text.append("列队"); break;
	case _DeviceRegain: text.append("回收"); break;
	case _DeviceLed: text.append("LED控制"); break;
	case _DeviceWaypoint: text.append("舞步"); break;
	case _DeviceCalibration: text.append("设备校准"); break;
	default: text.append("未定义操作"); break;
	}
	qInfo() << text;
	if (_DeviceTakeoffLocal == comand || _DevicePrepare == comand) {
		//起飞前检查所有设备状态
		//设备连接状态
		//设备电量
		//舞步已上传
		//无人机是否在初始位置附近
		//起飞时_DeviceTakeoffLocal检查已经准备起飞
		//TODO 需要考虑已上传舞步后又重新编辑舞步处理方式
#ifndef _DebugApp_
		if (0 == m_stationMap.count()) {
			m_bControlIng = false;
			_ShowErrorMessage(tr("基站未成功标定无法起飞"));
			return;
		}
#endif
		//先清空错误消息
		_MessageListClear;
		//记录出错设备名称
		QStringList listErrorNames;
		QStringList listCheck;
		for (int i = 0; i < ui.listWidget->count(); i++) {
			bool bTimeSync = true;
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			if (false == pDevice->isCheckDevice()) continue;
			QString name = pDevice->getName();
			listCheck.append(name);
			listErrorNames.append(name);
			if (false == pDevice->isConnectDevice()) {
				_ShowErrorMessage(name + tr("设备未连接无法起飞"));
				continue;
			}
#ifndef _DebugApp_
			if (pDevice->getCurrentStatus().battery < 50) {
				_ShowErrorMessage(name + tr("设备电量过低无法起飞"));
				continue;
			}
			if (false == pDevice->isUploadWaypointFinished()) {
				_ShowErrorMessage(name + tr("设备未上传舞步无法起飞"));
				continue;
			}
			if (pDevice->isUploadWaypointIng()) {
				_ShowErrorMessage(name + tr("正在上传舞步中，请稍后重试"));
				continue;
			}
			if (qAbs(pDevice->getX() - pDevice->getCurrentStatus().x) > _UAVStartLocation_) {
				_ShowErrorMessage(name + QString("设备X轴方向距离初始位置超过%1厘米").arg(_UAVStartLocation_));
				continue;
			}
			if (qAbs(pDevice->getY() - pDevice->getCurrentStatus().y) > _UAVStartLocation_) {
				_ShowErrorMessage(name + QString("设备Y轴方向距离初始位置超过%1厘米").arg(_UAVStartLocation_));
				continue;
			}
			if (m_timerSync.isActive()) {
				QMessageBox::information(this, tr("提示"), tr("定装授时正在进行中"));
				m_bControlIng = false;
				return;
			}
			if (false == pDevice->isTimeSyncFinished() || 0 == pDevice->getTimeSyncMSecsUTC()) {
				_ShowErrorMessage(name + tr("未成功定桩授时"));
				m_bControlIng = false;
				return;
			}
			//检查定桩授时是否同步，防止单独对一个无人机进行定桩授时
			for (int i = 0; i < ui.listWidget->count(); i++) {
				QListWidgetItem* pItem = ui.listWidget->item(i);
				if (!pItem) continue;
				QWidget* pWidget = ui.listWidget->itemWidget(pItem);
				if (!pWidget) continue;
				DeviceControl* pTemp = dynamic_cast<DeviceControl*>(pWidget);
				if (!pTemp) continue;
				if (false == pTemp->isCheckDevice()) continue;
				if (pDevice->getName() == pTemp->getName()) continue;
				if (false == pTemp->isTimeSyncFinished()) continue;
				qint64 n = pDevice->getTimeSyncMSecsUTC() - pTemp->getTimeSyncMSecsUTC();
				if (qAbs(n) >= _DeviceTimeLimits_) {
					qWarning() << "定桩授时不同步" << pDevice->getName() << pDevice->getTimeSyncMSecsUTC() << pTemp->getName() << pTemp->getTimeSyncMSecsUTC() << n;
					bTimeSync = false;
					break;
				}
			}
			if (false == bTimeSync) {
				_ShowErrorMessage(name + tr("定桩授时不同步，请重新对所有无人机进行定桩授时"));
				continue;
			}
#endif
			if (_DeviceTakeoffLocal == comand && false == pDevice->isPrepareTakeoff()) {
				_ShowErrorMessage(name + tr("设备未准备起飞"));
				continue;
			}
			//所有判断检查通过
			listErrorNames.removeOne(name);
		}
		//所有设备准备完成才可以起飞
		if (false == listErrorNames.isEmpty()) {
			m_bControlIng = false;
			QString qstrError = listErrorNames.join("、");
			QString error = qstrError + tr("检查出错，请重试");
			_ShowErrorMessage(error);
			QMessageBox::warning(this, tr("警告"), error);
			return;
		}
		if (listCheck.isEmpty()) {
			m_bControlIng = false;
			_ShowInfoMessage(tr("未选中无人机"));
			return;
		}

		if (_DeviceTakeoffLocal == comand) {
			//起飞前增加倒计时
			QWidget* pWidget = this;
			while (true) {
				QWidget* pTemp = dynamic_cast<QWidget*>(pWidget->parent());
				if (!pTemp) break;
				pWidget = pTemp;
			}
			QLabel label(pWidget);
			//设置窗体的背景色,这里的百分比就是透明度
			label.setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);color:#FF0000;font-size:100px;"));
			label.setFixedSize(pWidget->size());
			label.show();
			label.setAlignment(Qt::AlignCenter);
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
	}
	if (_DeviceTimeSync == comand) {
		//定桩授时之前检查所有设备连接状态		
		QStringList errorList;
		for (int i = 0; i < ui.listWidget->count(); i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			if (false == pDevice->isCheckDevice()) continue;
			QString qstrName = pDevice->getName();
			if (pDevice->isConnectDevice()) continue;
			errorList.append(qstrName);
		}
		if (false == errorList.isEmpty()) {
			m_bControlIng = false;
			QString error = errorList.join("、") + tr("未连接无法进行定桩授时");
			_ShowErrorMessage(error);
			QMessageBox::warning(this, tr("失败"), error);
			return;
		}
	}
	int x = 0;
	int y = 0;
	QStringList listCheck;
	QStringList listErrorDevice;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		if (false == pDevice->isCheckDevice()) continue;
		QString qstrName = pDevice->getName();
		listCheck.append(qstrName);
		int res = DeviceDataSucceed;
		QString qstrText;
		switch (comand)
		{
		case _DeviceTakeoffLocal:
			res = pDevice->Fun_MAV_CMD_NAV_TAKEOFF_LOCAL(0, 0, 0, 0, 0, 0, _TakeoffLocalHeight_, false);
			qstrText = tr("起飞");
			break;
		case _DeviceLandLocal:
			res = pDevice->Fun_MAV_CMD_NAV_LAND_LOCAL(0, 0, 0, 0, 0, 0, 0, false);
			qstrText = tr("降落");
			break;
		case _DeviceQuickStop:
			res = pDevice->Fun_MAV_QUICK_STOP(false);
			qstrText = tr("急停");
			break;
		case _DeviceTimeSync:
			qstrText = tr("定桩授时");
			res = pDevice->Fun_MAV_TimeSync();
			break;
		case _DevicePrepare:
			res = pDevice->Fun_MAV_CMD_DO_SET_MODE();
			qstrText = tr("准备起飞");
			break;
		case _DeviceQueue:
			//降落到初始位置
			res = pDevice->Fun_MAV_Defined_Queue(pDevice->getX(), pDevice->getY());
			qstrText = tr("列队");
			break;
		case _DeviceRegain:
		{
			//按顺序排列
			qstrText = tr("回收");
			int nInterval = 50;			//无人机间隔
			int c = getSpaceSize().x() / nInterval;
			if (i > 0) {
				int r = i % c;
				x = r * nInterval;
				if (0 == r) y += nInterval;
			}
			res = pDevice->Fun_MAV_Defined_Regain(x, y);
			break;
		}
		default:
			break;
		}
		qDebug() << QString("%1%2错误码%3").arg(qstrName).arg(qstrText).arg(res);
		if (_DeviceStatus::DeviceDataSucceed != res) {
			listErrorDevice.append(qstrName);
			_ShowErrorMessage(qstrName + qstrText + tr("出错:") + Utility::waypointMessgeFromStatus(comand, res));
		}
		if (_DevicePrepare == comand && _DeviceStatus::DeviceDataSucceed == res) {
			_ShowInfoMessage(qstrName + "起飞前准备完毕");
		}
	}

	if (listCheck.isEmpty()) {
		m_bControlIng = false;
		_ShowInfoMessage(tr("未选中无人机"));
		return;
	}
	//其他情况停止音乐播放
	if (_DeviceTakeoffLocal == comand) emit sigTakeoffFinished(true);
	else emit sigTakeoffFinished(false);
	if (_DeviceTimeSync == comand) {
		//定时查询授时是否全部返回
		if (m_timerSync.isActive())m_timerSync.stop();
		m_timerSync.setProperty("start", QDateTime::currentDateTime());
		m_timerSync.start(1000);
	}
	m_bControlIng = false;
	return;
}

QString DeviceManage::waypointComposeAndUpload(QString qstrProjectFile, bool upload)
{
	_MessageListClear

		//执行python中不能重复执行，防止崩溃
		static bool bComposeIng = false;
	if (bComposeIng) {
		_ShowInfoMessage("正在处理舞步中，请稍后重试");
		return "正在处理舞步中，请稍后重试";
	}
	bComposeIng = true;
	//上传航点过程中禁止操作无人机列表
	if (upload) setEnabled(false);
	//MAP用于统一发送航点信息到三维
	QMap<QString, QVector<NavWayPointData>> map;
	qDebug() << "准备生成舞步信息";
	QString qstrErrorNames;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		QString name = pDevice->getName();
		if (name.isEmpty()) continue;
		if (false == pDevice->isCheckDevice()) continue;
		QFileInfo infoProject(qstrProjectFile);
		QString qstrDevicePyFile = infoProject.path() + _ProjectDirName_ + name + _PyFileSuffix_;
		if (false == QFile::exists(qstrDevicePyFile)) continue;
		//如果手动编写python文件存在则使用手动编写python
		bool bMauanl = false;
		QString qstrManualFile = infoProject.path() + _ProjectDirName_ + name + _PyManualSuffix_;
		if (true == QFile::exists(qstrManualFile)) {
			qstrDevicePyFile = qstrManualFile;
			bMauanl = true;
			_ShowInfoMessage(name + tr("使用python代码编译舞步"));
		}
		QFile file(qstrDevicePyFile);
		if (!file.open(QIODevice::ReadOnly)) continue;
		QByteArray arrData = file.readAll();
		file.close();

		if (arrData.isEmpty()) {
			_ShowErrorMessage(name + tr("没有编写舞步"));
			qstrErrorNames.append("," + pDevice->getName());
			continue;
		}

		int index = arrData.indexOf("Fly_");
		if (bMauanl && index >= 0) {
			//积木块接口前缀Fly_,用于区分
			//PyImport_AppendInittab会全局添加到python内置表中，无法区分积木块与python编程调用库的区分，所以此处直接判断是否使用了积木块的接口
			//判断是否使用积木块的python代码
			_ShowErrorMessage(name + tr("python编程代码API使用错误"));
			qstrErrorNames.append("," + pDevice->getName());
			continue;
		}

		if (false == bMauanl) {
			//通过Python代码检查积木块连贯性，当包含空行时说明积木块没有连到一起
			bool bGap = false;
			if (file.open(QIODevice::ReadOnly)) {
				//判断是否有变量，有变量则删除后边的空白行
				QByteArray data = file.readAll();
				file.close();
				if (data.contains("= None\n\n")) {
					data = data.replace("= None\n\n", "= None\n");
					if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
						file.write(data);
						file.close();
					}
				}
			}
			if (file.open(QIODevice::ReadOnly)) {
				while (false == file.atEnd()) {
					QByteArray arrLine = file.readLine();
					//去除回车换行后为空则代表有空白行
					arrLine = arrLine.replace("\r", "").replace("\n", "");
					if (arrLine.isEmpty()) {
						bGap = true;
						break;
					}
				}
				file.close();
			}
			if (bGap) {
				_ShowErrorMessage(name + tr("编程积木块不连贯，请检查"));
				qstrErrorNames.append("," + pDevice->getName());
				continue;
			}
		}
		qInfo() << "开始处理舞步" << pDevice->getName() << bMauanl;
		//执行python脚本之前初始参数，用于检查无人机编程参数
		pythonThread.initParam(m_pointSpace.x(), m_pointSpace.y(), pDevice->getName(), pDevice->getX(), pDevice->getY(), !bMauanl);
		//生成舞步过程必须一个个生成，python交互函数是静态全局，所以同时只能执行一个设备生成舞步
		if (!pythonThread.compilePythonCode(arrData)) {
			//生成舞步失败
			_ShowErrorMessage(name + tr("解析舞步积木块失败"));
			qstrErrorNames.append("," + pDevice->getName());
			continue;
		}
		//TODO 等待python文件执行完成，此处需要优化，有可能造成死循环
		while (!pythonThread.isFinished()) {
			QApplication::processEvents();
		}
		int state = pythonThread.getLastState();
		if (PythonSuccessful != state) {
			_ShowErrorMessage(name + tr("舞步转换失败") + pythonThread.getErrorString(state));
			qstrErrorNames.append("," + pDevice->getName());
			continue;
		}
		QVector<NavWayPointData> data = pythonThread.getWaypointData();

		//保存航点数据到本地，便于查看
		QFile fileSvg(QApplication::applicationDirPath() + "/waypoint/" + name + ".csv");
		if (fileSvg.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			QTextStream text_stream(&fileSvg);
			text_stream.setCodec("gbk");
			QString title = "参数一,参数二,参数三,参数四,X,Y,Z,ID,说明";
			text_stream << title << "\r\n";
			for (int i = 0; i < data.count(); i++) {
				NavWayPointData wp = data.at(i);
				QString text = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9")
					.arg(wp.param1).arg(wp.param2).arg(wp.param3).arg(wp.param4).arg(wp.x).arg(wp.y).arg(wp.z).arg(wp.commandID)
					.arg(Utility::getWaypointDescribeFromID(wp.commandID));
				text_stream << text << "\r\n";
				qDebug() << name << QString("第%1条航点").arg(i) << text;
			}
			fileSvg.flush();
			fileSvg.close();
		}
		QString temp = QString("%1舞步数量%2").arg(name).arg(data.count());
		qInfo() << temp;
		//最少2条，第一条是设置的起始位置
		if (data.count() <= 1) {
			_ShowErrorMessage(name + tr("没有舞步信息"));
			qstrErrorNames.append("," + pDevice->getName());
			continue;
		}
		//超过无人机保存舞步数量
		if (data.count() > _WaypointMaxCount_) {
			_ShowErrorMessage(temp + "超出最大的上传数量" + QString::number(_WaypointMaxCount_));
			qstrErrorNames.append("," + pDevice->getName());
			continue;
		}
		NavWayPointData waypoint = data.back();
		if (_WaypointFlyLand != waypoint.commandID) {
			_ShowErrorMessage(name + tr("降落动作缺少或顺序错误"));
			qstrErrorNames.append("," + pDevice->getName());
			continue;
		}
		qDebug() << name << "航点数据" << data.count();
		//TODO 判断最低起飞高度 暂定为0
		if (0 > data.at(1).z) {
			_ShowErrorMessage(name + tr("起飞高度太低"));
			qstrErrorNames.append("," + pDevice->getName());
			continue;
		}
		//使用API编写时不处理动作时间组
		if (false == bMauanl) {
			//判断动作时间组是否存在
			QMap<QString, unsigned int> mapTimeGroup = pythonThread.getTimeGroup();
			if (mapTimeGroup.isEmpty()) {
				_ShowErrorMessage(name + tr("错误，没有使用动作时间组"));
				qstrErrorNames.append("," + pDevice->getName());
				continue;
			}
			//更新积木块文件里动作组中时间值
			QString qstrError = updateBlocklyData(name, mapTimeGroup);
			if (false == qstrError.isEmpty()) {
				_ShowErrorMessage(name + qstrError);
				qstrErrorNames.append("," + pDevice->getName());
				continue;
			}
		}
		//判断飞行动作是否超出音乐时长，在更新动作时间组以后判断，便于展示总耗时
		if (pythonThread.getFlyTotalTime() > m_nMusicMaxTime) {
			unsigned int minute = m_nMusicMaxTime / 60;
			unsigned int second = m_nMusicMaxTime % 60;
			_ShowErrorMessage(name + QString("飞行动作超出音乐时长，音乐总时长%1分%2秒").arg(minute).arg(second));
			qstrErrorNames.append("," + pDevice->getName());
			continue;
		}
		if (false == upload) {
			_ShowInfoMessage(QString("%1生成舞步完成，共%2条舞步").arg(name).arg(data.count()));
		}
		map.insert(name, data);
	}

	//先发送航点到三维，允许在航点有碰撞情况进行三维模拟
	sendWaypointTo3D(map);

	//根据航点检查碰撞，把总时以固定间隔分片，计算所有无人机所在位置，然后判断是否有无人机距离过近
	//当前毫秒时间，无人机位置
	QMap<unsigned int, QList<_MidwayPosition>> mapTime;
	//分片时间间隔，毫秒
	int nInterval = 10;
	QStringList keys = map.keys();
	//记录降落位置
	QList<_MidwayPosition> deviceFlyLandList;
	//分解所有无人机位置
	foreach(QString name, keys) {
		QVector<NavWayPointData> data = map.value(name);
		//初始位置
		NavWayPointData start = data.at(0);
		unsigned int nStartTime = start.param3 * 1000;
		for (int i = 1; i < data.count(); i++) {
			NavWayPointData end = data.at(i);
			//非航点数据不处理
			if (_WaypointFly != end.commandID && _WaypointFlyLand != end.commandID && _WaypointFlyTakeOff != end.commandID) continue;
			if (_WaypointFlyLand == end.commandID) {
				//降落时使用固定速度
				end.param3 = (start.z - end.z) / _WaypointLanding_;
				//记录降落位置，防止无人机降落时间不一样，无法判断降落时是否碰撞
				deviceFlyLandList.append(_MidwayPosition(name, end.blockid, end.x, end.y, end.z));
			}
			//总时长,毫秒
			int maxMillisecond = end.param3 * 1000;
			//按时间间隔切分航线，细分判断每个航点
			double temp = maxMillisecond / nInterval;
			int n = qRound(temp);
			for (int j = 1; j <= n; j++) {
				//直接使用整数，四舍五入精确到厘米
				int x = start.x + (end.x - start.x) * j / n;
				int y = start.y + (end.y - start.y) * j / n;
				int z = start.z + (end.z - start.z) * j / n;
				_MidwayPosition pos(name, end.blockid, x, y, z);
				unsigned int m = j * nInterval;
				if (m > maxMillisecond) m = maxMillisecond;
				unsigned int currentTime = nStartTime + m;
				QList<_MidwayPosition> temp = mapTime.value(currentTime);
				temp.append(pos);
				mapTime.insert(currentTime, temp);
			}
			start = end;
			nStartTime = nStartTime + start.param3 * 1000;
		}
	}
	//判断无人机在时间片段点距离是否过近
	QList<unsigned int> listTimeKeys = mapTime.keys();
	//已经判断过的重叠设备
	struct stProcessedDevice {
		unsigned int nEndOverlap;
		QString name1;
		QString name2;
		stProcessedDevice(unsigned int n, QString qstr1, QString qstr2) {
			nEndOverlap = n;
			name1 = qstr1;
			name2 = qstr2;
		}
	};
	QList<stProcessedDevice> listOverlapProcessed;
	//按时间片循环，分片时间间隔nInterval毫秒
	//因为存在每台无人机飞行时间不一致，所以先按时间片循环
	foreach(unsigned int current, listTimeKeys) {
		QList<_MidwayPosition> deviceList = mapTime.value(current);
		int i = 1;
		//循环判断无人机在当前时间current所在位置
		foreach(_MidwayPosition pos, deviceList) {
			QList<_MidwayPosition> residue = deviceList.mid(i, deviceList.length() - i);
			i++;
			//与当前无人机后边pos的无人机进行位置距离比较
			foreach(_MidwayPosition temp, residue) {
				//计算相互之间距离
				QString error;
				int d = getDistance(pos.x, pos.y, pos.z, temp.x, temp.y, temp.z);
				if (d < _UAVMinDistance_) {
					error = QString("%1在X:%2厘米 Y:%3厘米 Z:%4厘米位置处与%5在X:%6厘米 Y:%7厘米 Z:%8厘米位置处距离小于%9厘米有碰撞风险")
						.arg(pos.name).arg(pos.x).arg(pos.y).arg(pos.z).arg(temp.name).arg(temp.x).arg(temp.y).arg(temp.z).arg(_UAVMinDistance_);
				}
				//判断Z轴是否有重叠
				int z = getDistance(pos.x, pos.y, 0, temp.x, temp.y, 0);
				//两台无人机相互重叠范围 厘米
				int nOverlapLenght = 30;
				if (z < nOverlapLenght) {
					bool bProcessed = false;
					if (false == listOverlapProcessed.isEmpty()) {
						//时间段内是否判断过重叠
						foreach(stProcessedDevice processed, listOverlapProcessed) {
							if (processed.nEndOverlap > current && processed.name1 == pos.name && processed.name2 == temp.name) {
								bProcessed = true;
								break;
							}
						}
					}
					if (false == bProcessed) {
						bool bOverlap = true;
						int minute = current / 1000 / 60;
						int second = current / 1000 % 60;
						int millisecond = current % 1000;
						QString t = QString("当%1分%2秒%3毫秒时").arg(minute).arg(second).arg(millisecond);
						QString text = QString("%1位置[X:%2 Y:%3 Z:%4]与%5位置[X:%6 Y:%7 Z:%8]开始重叠")
							.arg(pos.name).arg(pos.x).arg(pos.y).arg(pos.z).arg(temp.name).arg(temp.x).arg(temp.y).arg(temp.z);
						//qInfo() << t + text;

						//此时两台无人机Z轴重叠，需要判断时间，当持续重叠时则警告
						int index = listTimeKeys.indexOf(current);
						for (int i = 0; i < (1000 / nInterval); i++) {
							//判断1000毫秒内是否持续重叠
							if (false == bOverlap) break;
							index++;	//找到下一个时间片
							int count = listTimeKeys.count();
							if (index >= count) break;
							unsigned int nNextTime = listTimeKeys.at(index);
							//下一个时间片所有无人机所在位置
							QList<_MidwayPosition> listNext = mapTime.value(nNextTime);
							_MidwayPosition w = pos;
							foreach(_MidwayPosition m, listNext) {
								if (pos.name == m.name) {
									w = m;
									continue;
								}
								if (temp.name == m.name) {
									//判断是否持续重叠
									int z = getDistance(w.x, w.y, 0, m.x, m.y, 0);
									if (z > nOverlapLenght) {
										//已经解除重叠，直接跳出判断
										bOverlap = false;
										listOverlapProcessed.append(stProcessedDevice(nNextTime, w.name, m.name));
										int minute = nNextTime / 1000 / 60;
										int second = nNextTime / 1000 % 60;
										int millisecond = nNextTime % 1000;
										QString t = QString("当%1分%2秒%3毫秒时").arg(minute).arg(second).arg(millisecond);
										QString text = QString("%1位置[X:%2 Y:%3 Z:%4]与%5位置[X:%6 Y:%7 Z:%8]解除重叠")
											.arg(w.name).arg(w.x).arg(w.y).arg(w.z).arg(m.name).arg(m.x).arg(m.y).arg(m.z);
										//qInfo() << t + text << "持续重叠时间" << nNextTime - current;
										break;
									}
								}
							}
						}
						if (bOverlap) {
							error = QString("%1在X:%2厘米 Y:%3厘米 Z:%4厘米位置处与%5在X:%6厘米 Y:%7厘米 Z:%8厘米位置处有上下重叠风险")
								.arg(pos.name).arg(pos.x).arg(pos.y).arg(pos.z).arg(temp.name).arg(temp.x).arg(temp.y).arg(temp.z);
						}
					}
				}
				if (!error.isEmpty()) {
					int minute = current / 1000 / 60;
					int second = current / 1000 % 60;
					int millisecond = current % 1000;
					QString t = QString("当%1分%2秒%3毫秒时").arg(minute).arg(second).arg(millisecond);
					_ShowErrorMessage(t + error);
					//定位当前设备的WEB积木块中，否则显示最前边
					QString qstrCurrentNmae = getCurrentDeviceName();
					if (qstrCurrentNmae == temp.name) emit sigBlockFlicker(temp.blockid);
					else emit sigBlockFlicker(pos.blockid);
					bComposeIng = false;
					setEnabled(true);
					return "，" + pos.name + "，" + temp.name;
				}
			}
		}
	}
	//判断降落过程是否有碰撞情况
	if (deviceFlyLandList.count() > 1) {
		_MidwayPosition pos = deviceFlyLandList.at(0);
		for (int i = 1; i < deviceFlyLandList.count(); i++) {
			_MidwayPosition temp = deviceFlyLandList.at(i);
			int d = getDistance(pos.x, pos.y, pos.z, temp.x, temp.y, temp.z);
			if (d < _UAVMinDistance_) {
				QString error = QString("%1与%2在降落过程中存在碰撞风险").arg(pos.name).arg(temp.name);
				_ShowErrorMessage(error);
				//定位当前设备的WEB积木块中，否则显示最前边
				QString qstrCurrentNmae = getCurrentDeviceName();
				if (qstrCurrentNmae == temp.name) emit sigBlockFlicker(temp.blockid);
				else emit sigBlockFlicker(pos.blockid);
				bComposeIng = false;
				setEnabled(true);
				return "，" + pos.name + "，" + temp.name;
			}
		}
	}
	qInfo() << "生成舞步完成";
	//判断是否需要上传舞步
	if (upload) {
		//上传舞步前检查无人机标签是否用重复，所有已连接设备都要检查，标签重复会造成定位错误
		QMap<int, QString> mapTags;
		for (int i = 0; i < ui.listWidget->count(); i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			int n = pDevice->getTag();
			if (n < 0) continue;
			if (mapTags.contains(n)) {
				//有重复的标签
				bComposeIng = false;
				setEnabled(true);
				QString error = QString("%1与%2设备标签重复，请关闭或修改后重新").arg(mapTags.value(n)).arg(pDevice->getName());
				_ShowErrorMessage(error);
				return error;
			}
			mapTags.insert(n, pDevice->getName());
		}
		//上传舞步到无人机
		for (int i = 0; i < ui.listWidget->count(); i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			QString name = pDevice->getName();
			if (name.isEmpty()) continue;
			if (false == pDevice->isCheckDevice()) continue;
#ifndef _DebugApp_
			if (false == pDevice->isConnectDevice()) {
				_ShowErrorMessage(name + tr("设备没有连接无法上传舞步"));
				continue;
			}
			if (qAbs(pDevice->getX() - pDevice->getCurrentStatus().x) > _UAVStartLocation_) {
				_ShowErrorMessage(name + QString("设备X轴方向距离初始位置超过%1厘米，无法上传舞步").arg(_UAVStartLocation_));
				continue;
			}
			if (qAbs(pDevice->getY() - pDevice->getCurrentStatus().y) > _UAVStartLocation_) {
				_ShowErrorMessage(name + QString("设备Y轴方向距离初始位置超过%1厘米，无法上传舞步").arg(_UAVStartLocation_));
				continue;
			}
			//上传舞步到飞控之前检查是否进行三维仿真
			qInfo() << QString("三维仿真是否完成:%1").arg(m_b3DFinished);
			if (false == m_b3DFinished) {
				bComposeIng = false;
				QString error = tr("三维仿真未完成，无法上传舞步到无人机");
				_ShowErrorMessage(error);
				QMessageBox::warning(this, tr("警告"), error);
				setEnabled(true);
				return error;
			}
			qInfo() << "三维仿真中碰撞设备" << m_map3DCollision;
			if (false == m_map3DCollision.isEmpty()) {
				QString text;
				QStringList list = m_map3DCollision.keys();
				foreach(QString name, list) {
					text.append(name + "，");
				}
				text.append("舞步存在碰撞风险，请检查后重试");
				bComposeIng = false;
				_ShowErrorMessage(text);
				QMessageBox::warning(this, "警告", text);
				setEnabled(true);
				return text;
			}
#endif
			//上传航点到飞控
			qInfo() << "准备上传舞步到飞控";
			int status = pDevice->DeviceMavWaypointStart(map.value(name));
			if (_DeviceStatus::DeviceDataSucceed == status) {
				_ShowInfoMessage(name + "开始舞步上传");
#ifdef _UseUWBData_
				while (pDevice->isUploadWaypointIng()){
					//等待航点航点上传完成后执行下一个飞机上传航点程序
					//因此连续发送会造成串口数据阻塞,造成后续无人机一直处于等待回应状态
					QApplication::processEvents();
				}
#endif
			}
			else {
				qstrErrorNames.append("," + pDevice->getName());
				_ShowErrorMessage(name + "上传舞步失败" + Utility::waypointMessgeFromStatus(_DeviceWaypoint, status));
			}
		}
	}
	bComposeIng = false;
	setEnabled(true);
	return qstrErrorNames;
}

void DeviceManage::setCurrentMusicPath(QString filePath, QPixmap pixmap)
{
	m_qstrMusicFile = filePath;
	m_pixmapMusic = pixmap;
	if (!QFile::exists(filePath)) return;
	QFileInfo info(filePath);
	QString qstrPixmapPath = info.path() + "/music.png";
	if (false == pixmap.save(qstrPixmapPath)) {
		qDebug() << "保存音乐波形图片失败";
		return;
	}
}

void DeviceManage::sendWaypointTo3D(QMap<QString, QVector<NavWayPointData>> map)
{
	if (!m_p3dTcpSocket) return;
	qDebug() << "准备发送初始化信息到三维";
	QJsonObject obj3dmsg;
	obj3dmsg.insert(_Ver_, _VerNum_);
	obj3dmsg.insert(_Tag_, _TabName_);
	obj3dmsg.insert(_ID_, _3dDeviceInit);
	obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	obj3dmsg.insert("x", getSpaceSize().x());
	obj3dmsg.insert("y", getSpaceSize().y());
	//无人机初始位置
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
	//基站坐标
	QJsonArray arrStation;
	if (m_stationMap.isEmpty()) {
		//没连接基站时使用默认位置
		QJsonObject obj;
		obj.insert("name", "A0");
		obj.insert("x", 0);
		obj.insert("y", 0);
		arrStation.append(obj);
		obj.insert("name", "A1");
		obj.insert("x", getSpaceSize().x());
		obj.insert("y", 0);
		arrStation.append(obj);
		obj.insert("name", "A2");
		obj.insert("x", getSpaceSize().x());
		obj.insert("y", getSpaceSize().y());
		arrStation.append(obj);
		obj.insert("name", "A3");
		obj.insert("x", 0);
		obj.insert("y", getSpaceSize().y());
		arrStation.append(obj);
		obj.insert("name", "A4");
		obj.insert("x", getSpaceSize().x() / 2);
		obj.insert("y", 0);
		arrStation.append(obj);
		obj.insert("name", "A5");
		obj.insert("x", getSpaceSize().x() / 2);
		obj.insert("y", getSpaceSize().y());
		arrStation.append(obj);
	}
	else {
		QStringList keys = m_stationMap.keys();
		foreach(QString name, keys) {
			QJsonObject obj;
			obj.insert("name", name);
			obj.insert("x", m_stationMap.value(name).x());
			obj.insert("y", m_stationMap.value(name).y());
			arrStation.append(obj);
		}
	}
	obj3dmsg.insert("station", arrStation);
	//音乐文件
	QString filePath = m_qstrMusicFile;
	QPixmap pixmap = m_pixmapMusic;
	if (!QFile::exists(filePath)) {
		qWarning() << "音乐文件不存在";
		return;
	}
	QFileInfo info(filePath);
	QString qstrPixmapPath = info.path() + "/music.png";
	if (!pixmap.save(qstrPixmapPath)) {
		qWarning() << "保存音乐波形图片失败";
		return;
	}
	QJsonObject jsonMusic;
	jsonMusic.insert("file", filePath);
	jsonMusic.insert("image", qstrPixmapPath);
	obj3dmsg.insert("music", jsonMusic);
	//航点列表
	QJsonArray jsonWaypoint;
	QStringList listNmae = map.keys();
	foreach(QString name, listNmae) {
		QVector<NavWayPointData> data = map[name];
		QJsonObject objDevice;
		objDevice.insert("name", name);
		QJsonArray arrWaypoint;

		//TODO 暂时定义默认飞行速度
		int speed = 60;
		//飞行总时间 毫秒
		int timesum = 0;
		int lastX = 0;
		int lastY = 0;
		int lastZ = 0;
		int angle = 0;
		//int color = 0;
		int red = 0;
		int green = 0;
		int blue = 0;
		int status = 0;
		for (int i = 0; i < data.count(); i++) {
			if (0 == i) {
				lastX = data.at(i).x;
				lastY = data.at(i).y;
				lastZ = data.at(i).z;
			}
			NavWayPointData waypoint = data.at(i);
			//非航点信息转换航点数据
			if (_WaypointSpeed == waypoint.commandID) {
				//设置飞行速度
				speed = waypoint.param1;
				if (speed <= 0) speed = 60;
				waypoint.param1 = 0;
				waypoint.commandID = _WaypointFly;
			}
			else if (_WaypointHover == waypoint.commandID) {
				waypoint.commandID = _WaypointFly;
			}
			else if (_WaypointRevolve == waypoint.commandID) {
				angle += waypoint.param1;
				if (angle >= 360) angle = angle % 360;
				//旋转用时1秒
				waypoint.param1 = 1;
				waypoint.x = lastX;
				waypoint.y = lastY;
				waypoint.z = lastZ;
				waypoint.commandID = _WaypointFly;
			}
			else if (_WaypointStart == waypoint.commandID) {
				waypoint.commandID = _WaypointFly;
				red = green = 0;
				blue = 255;
				status = 8;	//开始时点亮LED
				if (waypoint.param3 > 0) {
					//三维中需要第0秒为初始位置
					//当延时起飞时把提前插入0秒时位置
					int x = waypoint.x;
					int y = waypoint.y;
					int z = waypoint.z;
					QList<QVariant> value;
					value << timesum << red << green << blue << status << angle << x << y << z << 16;
					arrWaypoint.append(QJsonArray::fromVariantList(value));
				}
				waypoint.param1 = waypoint.param2 = waypoint.param4 = 0;
			}
			else if (_WaypointLedColor == waypoint.commandID) {
				//color = QString::number(waypoint.param1).toInt() * 1000 * 1000 + QString::number(waypoint.param2).toInt() * 1000 + QString::number(waypoint.param3).toInt();
				red = waypoint.param1;
				green = waypoint.param2;
				blue = waypoint.param3;
				waypoint.param1 = waypoint.param2 = waypoint.param3 = waypoint.param4 = 0;
				waypoint.x = lastX;
				waypoint.y = lastY;
				waypoint.z = lastZ;
				waypoint.commandID = _WaypointFly;
			}
			else if (_WaypointLedStatus == waypoint.commandID) {
				status = waypoint.param1;
				waypoint.param1 = waypoint.param2 = waypoint.param3 = waypoint.param4 = 0;
				waypoint.x = lastX;
				waypoint.y = lastY;
				waypoint.z = lastZ;
				waypoint.commandID = _WaypointFly;
			}
			else if (_WaypointFlyLand == waypoint.commandID) {
				status = 9; //降落熄灭LED
				red = green = blue = 0;
				int x = waypoint.x;
				int y = waypoint.y;
				int z = waypoint.z;
				int d = getDistance(lastX, lastY, lastZ, x, y, z);
				waypoint.param3 = d / _WaypointLanding_;
				waypoint.commandID = _WaypointFly;
			}
			else if (_WaypointFlyTakeOff == waypoint.commandID) {
				waypoint.commandID = _WaypointFly;
			}

			if (_WaypointFly != waypoint.commandID) continue;
			int x = waypoint.x;
			int y = waypoint.y;
			int z = waypoint.z;
			//旋转角度累加
			angle += waypoint.param4;
			if (i == 0) {
				//第一个航点需要使用默认起飞位置
				DeviceControl* pDevcie = getDeviceFromName(name);
				if (!pDevcie) continue;
				x = pDevcie->getX();
				y = pDevcie->getY();
			}
			//计算空间点距离
			int d = getDistance(lastX, lastY, lastZ, x, y, z);
			//计算飞行时间 时间使用毫秒单位
			int time = d * 1000 / speed;
			time = waypoint.param3 * 1000;
			timesum += time;
			QList<QVariant> value;
			value << timesum << red << green << blue << status << angle << x << y << z << 16;
			arrWaypoint.append(QJsonArray::fromVariantList(value));
			lastX = x;
			lastY = y;
			lastZ = z;
		}
		objDevice.insert("list", arrWaypoint);
		jsonWaypoint.append(objDevice);
	}
	obj3dmsg.insert("waypoint", jsonWaypoint);
	sendMessageTo3D(obj3dmsg);
}

void DeviceManage::updateMusicTime(unsigned int time)
{
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		pDevice->setCurrentTime(time);
	}
}

void DeviceManage::showFirmwareDialog()
{
	if (!m_pFirmwareDialog) {
		m_pFirmwareDialog = new FirmwareDialog(this);
	}
	m_pFirmwareDialog->setDeviceNameList(getDeviceNameList());
	m_pFirmwareDialog->exec();
}

void DeviceManage::setCrrentProject(QString path)
{
	m_qstrCurrentProjectFile = path;
}

void DeviceManage::reset3DStatus()
{
	qInfo() << "清空三维仿真记录";
	m_b3DFinished = false;
	ui.label3DStatus->setText("<font color=#FF0000>三维仿真未完成</font>");
	m_map3DCollision.clear();
}

bool DeviceManage::eventFilter(QObject* watched, QEvent* event)
{
	if (!isEnabled()) return false;
	if (ui.listWidget == watched) {
		//设备列表菜单
		if (QEvent::ContextMenu != event->type()) return false;
		QListWidgetItem* pItem = ui.listWidget->itemAt(ui.listWidget->mapFromGlobal(QCursor::pos()));
		if (pItem) {
			//当前在上传舞步过程中禁用右键菜单，防止进行起飞降落操作
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (pWidget) {
				DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
				if (pDevice && pDevice->isUploadWaypointIng()) {
					return false;
				}
			}
			if (m_bDebug) {
				m_pMenu->addAction(m_pActionMagnetismOpen);
				m_pMenu->addAction(m_pActionMagnetismClose);
				m_pMenu->addAction(m_pActionGyro);
				m_pMenu->addAction(m_pActionBaro);
				m_pMenu->addAction(m_pActionDebug);
			}
			else {
				m_pMenu->removeAction(m_pActionMagnetismOpen);
				m_pMenu->removeAction(m_pActionMagnetismClose);
				m_pMenu->removeAction(m_pActionGyro);
				m_pMenu->removeAction(m_pActionBaro);
				m_pMenu->removeAction(m_pActionDebug);
			}
			m_pMenu->exec(QCursor::pos());
		}
		else {
			//点击空白区域时判断是否已经复制设备，如果有复制设备则显示粘贴菜单
			if (false == m_qstrCopyName.isEmpty()) {
				qInfo() << "有可粘贴设备" << m_qstrCopyName;
				m_pActionPaste->setText(tr("粘贴:") + m_qstrCopyName);
				m_pMenuPaste->exec(QCursor::pos());
			}
		}
	}
	return false;
}

void DeviceManage::resizeEvent(QResizeEvent* event)
{
	if (width() > ui.widgetBackgroundMain->width()) {
		ui.widgetPreflightCheck->setVisible(true);
		ui.widgetAdd->setVisible(false);
		ui.widgetButton->setVisible(false);
	}
	else {
		ui.widgetPreflightCheck->setVisible(false);
		ui.widgetAdd->setVisible(true);
		ui.widgetButton->setVisible(true);
	}
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		pDevice->enableControl(ui.btnAddDevice->isEnabled());
	}
}

void DeviceManage::keyReleaseEvent(QKeyEvent* keyEvent)
{
	if (!keyEvent) return;
	if (keyEvent->key() == Qt::Key_Q) {
		m_bDebug = !m_bDebug;
	}
}

void DeviceManage::on3dNewConnection()
{
	if (!m_p3dTcpServer) return;
	if (m_p3dTcpSocket) {
		m_p3dTcpSocket->disconnect();
		m_p3dTcpSocket->close();
	}
	m_p3dTcpSocket = m_p3dTcpServer->nextPendingConnection();
	qDebug() << "三维窗口新连接" << m_p3dTcpSocket;
	connect(m_p3dTcpSocket, &QTcpSocket::readyRead, [this]() {
		QByteArray arrData = m_p3dTcpSocket->readAll();
		qDebug() << "收到3D消息:" << arrData;
		//解析三维消息并把发送记录中的消息删除
		QList<QByteArray> list = arrData.split(_MsgTail);
		foreach(QByteArray data, list) {
			data = data.replace(_MsgHead, "");
			analyzeMessageFrom3D(data);
		}
		});
	connect(m_p3dTcpSocket, &QTcpSocket::disconnected, [this]() {
		emit sig3DDialogStatus(false);
		qDebug() << "三维窗口关闭";
		m_p3dTcpSocket = nullptr;
		m_map3DMsgRecord.clear();
		if (m_timerUpdateStatus.isActive()) m_timerUpdateStatus.stop();
		if (m_timerMessage3D.isActive()) m_timerMessage3D.stop();
		if (false == m_b3DFinished) {
			//未收到三维窗口音乐进度完成消息，代表三维仿真未完成，无法上传舞步
			_ShowErrorMessage("三维仿真未完成，请重试");
		}
		else {
			_ShowInfoMessage("三维仿真结束");
			ui.label3DStatus->setText("<font color=#467FC1>三维仿真已完成</font>");
		}
		});
	emit sig3DDialogStatus(true);
}

void DeviceManage::onTimeout3DMessage()
{
	QList<int> listKey = m_map3DMsgRecord.keys();
	if (listKey.isEmpty())  return;
	qDebug() << "重发三维消息" << listKey;
	foreach(int id, listKey) {
		QJsonObject obj = m_map3DMsgRecord[id];
		sendMessageTo3D(obj);
	}
}

void DeviceManage::onTimeoutSync()
{
	//指令下发超时时间9秒，如果全成功，并且时间差值在限定值内则提示成功
	QDateTime startTime = m_timerSync.property("start").toDateTime();
	QDateTime currentTime = QDateTime::currentDateTime();
	int n = startTime.secsTo(currentTime);
	if (qAbs(n) > 9) {
		//判断超时的情况
		if (m_timerSync.isActive()) m_timerSync.stop();
		//清空所有无人机定桩授时状态
		for (int i = 0; i < ui.listWidget->count(); i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			pDevice->clearTimeSyncStatus();
		}
		QMessageBox::warning(this, tr("警告"), tr("定桩授时超时"));
	}
	else {
		//先判断定桩授时操作是否完成
		bool bAllFinished = false;
		for (int i = 0; i < ui.listWidget->count(); i++) {
			bAllFinished = true;
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			if (false == pDevice->isCheckDevice()) continue;
			if (false == pDevice->isTimeSyncFinished()) bAllFinished = false;
		}
		qint64 nMinTime = 0;
		qint64 nMaxTime = 0;
		if (bAllFinished) {
			//所有无人机定桩授时完成，判断同步
			if (m_timerSync.isActive()) m_timerSync.stop();
			for (int i = 0; i < ui.listWidget->count(); i++) {
				bAllFinished = true;
				QListWidgetItem* pItem = ui.listWidget->item(i);
				if (!pItem) continue;
				QWidget* pWidget = ui.listWidget->itemWidget(pItem);
				if (!pWidget) continue;
				DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
				if (!pDevice) continue;
				if (false == pDevice->isCheckDevice()) continue;
				if (0 == pDevice->getTimeSyncMSecsUTC()) {
					//定桩授时有失败情况则全部失败
					for (int i = 0; i < ui.listWidget->count(); i++) {
						QListWidgetItem* pItem = ui.listWidget->item(i);
						if (!pItem) continue;
						QWidget* pWidget = ui.listWidget->itemWidget(pItem);
						if (!pWidget) continue;
						DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
						if (!pDevice) continue;
						pDevice->clearTimeSyncStatus();
					}
					QMessageBox::warning(this, tr("警告"), tr("定桩授时失败"));
					return;
				}
				if (0 == nMinTime) nMinTime = pDevice->getTimeSyncMSecsUTC();
				nMinTime = qMin(pDevice->getTimeSyncMSecsUTC(), nMinTime);
				nMaxTime = qMax(pDevice->getTimeSyncMSecsUTC(), nMaxTime);
			}
			qint64 n = qAbs(nMaxTime - nMinTime);
			if (n > _DeviceTimeLimits_) {
				QMessageBox::warning(this, tr("警告"), tr("定桩授时不同步"));
				return;
			}
			QMessageBox::information(this, tr("提示"), tr("定桩授时完成"));
		}
	}
}

void DeviceManage::onUpdateStatusTo3D()
{
	//定时向三维发送无人机状态信息
	QJsonObject obj3dmsg;
	obj3dmsg.insert(_Ver_, _VerNum_);
	obj3dmsg.insert(_Tag_, _TabName_);
	obj3dmsg.insert(_ID_, _3dDeviceLocation);
	obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	QJsonArray jsonArr;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		//设备连接后才更新实时位置
		if (!pDevice->isConnectDevice()) continue;
		_stDeviceCurrentStatus status = pDevice->getCurrentStatus();
		QJsonObject device;
		device.insert("name", pDevice->getName());
		device.insert("x", status.x);
		device.insert("y", status.y);
		device.insert("z", status.z);
		device.insert("pitch", status.pitch);
		device.insert("yaw", status.yaw);
		device.insert("roll", status.roll);
		device.insert("led", status.led);
		device.insert("battery", status.battery);
		jsonArr.append(device);
	}
	if (0 == jsonArr.count()) return;
	obj3dmsg.insert(_Data_, jsonArr);
	sendMessageTo3D(obj3dmsg);
}

void DeviceManage::onRemoveDevice(QString name)
{
	//删除前提示确认
	QMessageBox::StandardButton button = QMessageBox::question(this, tr("删除"), QString("确定删除%1设备，删除后无法恢复？").arg(name));
	if (QMessageBox::Yes != button) return;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		QString temp = pDevice->getName();
		if (temp != name) continue;
		ui.listWidget->takeItem(i);
		if (m_qstrCopyName == name) m_qstrCopyName.clear();
		emit deviceRemoveFinished(name);
		return;
	}

}

void DeviceManage::onWaypointFinished(QString name, bool success, QString text)
{
	emit sigWaypointFinished(name, success, text);
}

void DeviceManage::onSendDataToUWB(unsigned int tag, QByteArray data)
{
#ifdef _UseUWBData_
	m_pUWBStation->appendData(tag, data);
#endif
}

void DeviceManage::onUWBConnectStatus(bool connect, QString error)
{
	static QString qstrLast = "";
	if (qstrLast == error) return;
	qstrLast = error;
	if (connect) {
		_ShowInfoMessage(error);
		ui.labelUWBStatus->setStyleSheet("background-color:#00FF00;border-radius:6px;");
	} else {
		_ShowErrorMessage(error);
		ui.labelUWBStatus->setStyleSheet("background-color:#FF0000;border-radius:6px;");
	}
}

void DeviceManage::onUWBReceiveData(QList<_ReadyData> list)
{
	foreach(_ReadyData data, list) {
		for (int i = 0; i < ui.listWidget->count(); i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			if(pDevice->getIP().toInt() != data.tag) continue;
			pDevice->receiveUWBData(data.tag, data.data);
		}
	}
}

void DeviceManage::onActionLocateDevice()
{
	DeviceControl* pDevice = getCurrentDevice();
	if (nullptr == pDevice) return;
	if (false == pDevice->isConnectDevice()) {
		QMessageBox::warning(this, tr("警告"), tr("设备未连接，无法定位"));
		return;
	}
	if (false == m_timerLocate.isActive()) m_timerLocate.start(1000);
	int nStartX = pDevice->getX();
	int nStartY = pDevice->getY();
	_stDeviceCurrentStatus current = pDevice->getCurrentStatus();
	if (qAbs(nStartX - current.x) > _UAVStartLocation_ || qAbs(nStartY - current.y) > _UAVStartLocation_) {
		//超出误差范围，播放警告音
		QApplication::beep();
	}
	else {
		m_timerLocate.stop();
	}
}

void DeviceManage::onActionPaste()
{
	qInfo() << "粘贴设备:" << m_qstrCopyName;
	QString qstrNewNmae = getNewDefaultName();
	//复制已存在的blockly文件
	QFileInfo info(m_qstrCurrentProjectFile);
	QString path = info.path();
	QString qstrDeviceBlocklyFile = path + _ProjectDirName_ + m_qstrCopyName + _BlcoklyFileSuffix_;
	if (false == QFile::exists(qstrDeviceBlocklyFile)) {
		_ShowErrorMessage(qstrNewNmae + "源文件错误无法粘贴");
		return;
	}
	QString qstrNewBlocklyFile = path + _ProjectDirName_ + qstrNewNmae + _BlcoklyFileSuffix_;
	QFile::copy(qstrDeviceBlocklyFile, qstrNewBlocklyFile);
	QPoint point = getNewDevicePoint();
	QString qstrError = addDevice(qstrNewNmae, "", point.x(), point.y());
	if (false == qstrError.isEmpty()) {
		_ShowErrorMessage(qstrError);
		return;
	}
	m_qstrCopyName.clear();
}

void DeviceManage::onUpdateMusicMaxTime(unsigned int time)
{
	m_nMusicMaxTime = time;
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

DeviceControl* DeviceManage::getDeviceFromName(QString name)
{
	if (name.isEmpty()) return nullptr;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		if (pDevice->getName() != name) continue;
		return pDevice;
	}
	return nullptr;
}

void DeviceManage::sendMessageTo3D(QJsonObject json3d)
{
	if (nullptr == m_p3dTcpSocket) return;
	int id = json3d.value(_ID_).toInt();
	if (id != _3dDeviceLocation) {
		//使用MAP模式，相同消息只保留最后一条
		if (!m_timerMessage3D.isActive()) m_timerMessage3D.start(30 * 1000);
		//TODO 调试暂时去掉三维消息重发
		//m_map3DMsgRecord.insert(id, json3d);
	}
	QJsonDocument document(json3d);
	QByteArray msg3d = document.toJson(QJsonDocument::Compact);
	qDebug() << "3dmessage" << json3d;
	//消息头部增加标识，尾部增加标识
	msg3d.prepend(_MsgHead);
	msg3d.append(_MsgTail);
	m_p3dTcpSocket->write(msg3d);
}

void DeviceManage::analyzeMessageFrom3D(QByteArray data)
{
	//解析三维返回消息
	QJsonParseError jsonError;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &jsonError);
	if (QJsonParseError::NoError != jsonError.error || jsonDoc.isEmpty() || !jsonDoc.isObject()) {
		return;
	}
	QJsonObject jsonObj = jsonDoc.object();
	//记录上一次消息，仿真三维仿真重复发送初始消息
	static QJsonObject lastJson;
	if (lastJson == jsonObj) return;
	lastJson = jsonObj;
	int id = jsonObj.value(_ID_).toInt();
	qDebug() << "处理三维消息" << id;
	if (m_map3DMsgRecord.contains(id)) {
		m_map3DMsgRecord.remove(id);
		if (m_map3DMsgRecord.isEmpty()) m_timerMessage3D.stop();
	}
	if (id == _3dDeviceInit) {
		reset3DStatus();
		//收到初始化回应后启动实时位置定时
		if (!m_timerUpdateStatus.isActive()) m_timerUpdateStatus.start(1000);
	}
	else if (_3dDeviceCollision == id) {
		//暂时不处理三维中的碰撞情况
		////三维仿真中发生无人机碰撞
		//qDebug() << "三维仿真中发生无人机碰撞" << jsonObj;
		//QString name = jsonObj.value("name").toString();
		//QStringList list = getDeviceNameList();
		//if (list.contains(name)) {
		//	m_map3DCollision.insert(name, true);
		//}
	}
	else if (_3dDeviceFinished == id) {
		qDebug() << "三维仿真结束" << jsonObj;
		m_b3DFinished = true;
	}
	else {
		qWarning() << "未知三维消息";
	}
}

int DeviceManage::getDistance(int x1, int y1, int z1, int x2, int y2, int z2)
{
	int d = qSqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2));
	return d;
}

void DeviceManage::deviceCalibration()
{
	QAction* pAction = dynamic_cast<QAction*>(sender());
	if (nullptr == pAction) return;
	int n = pAction->property("Calibration").toInt();
	DeviceControl* pDevice = getCurrentDevice();
	if (nullptr == pDevice) return;
	if (false == pDevice->isConnectDevice()) {
		QMessageBox::warning(this, tr("提示"), tr("设备未连接，无法校准"));
		return;
	}
	QMessageBox::StandardButton button = QMessageBox::question(this, tr("询问"), tr("校准开始后无法操控无人机，是否现在开始校准?"));
	if (QMessageBox::StandardButton::Yes != button) return;
	CalibrationDialog* pDialog = new CalibrationDialog(n, pDevice, this);
	pDialog->addLogToBrowser(pDevice->getName() + tr("：校准开始"));
	int res = DeviceDataSucceed;
	switch (n) {
	case _Gyro: res = pDevice->Fun_MAV_CALIBRATION(1, 0, 0, 0, 0, 0, 0); break;
	case _Magnetometer: res = pDevice->Fun_MAV_CALIBRATION(0, 1, 0, 0, 0, 0, 0); break;
	case _Accelerometer: res = pDevice->Fun_MAV_CALIBRATION(0, 0, 0, 0, 1, 0, 0); break;
	case _Baro: res = pDevice->Fun_MAV_CALIBRATION(0, 0, 0, 0, 0, 0, 1); break;
	}
	if (DeviceDataSucceed != res) {
		_ShowErrorMessage(pDevice->getName() + tr("校准") + Utility::waypointMessgeFromStatus(_DeviceCalibration, res));
		return;
	}
	pDialog->exec();
}

QString DeviceManage::updateBlocklyData(QString name, QMap<QString, unsigned int> mapTime)
{
	if (m_qstrCurrentProjectFile.isEmpty() || name.isEmpty()) return QString();
	QFileInfo info(m_qstrCurrentProjectFile);
	QString path = info.path();
	QString filepath = QString("%1%2%3.blockly").arg(path).arg(_ProjectDirName_).arg(name);

	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(filepath).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) {
		qWarning() << "无法打开blockly文件更新动作组时间" << error << filepath;
		return "内部错误，无法更新动作组时间";
	}
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) {
		qWarning() << "无法更新blockly文件中动作组时间，缺少必要ROOT节点" << filepath;
		return "内部错误，无法更新动作组时间";
	}
	if (XMLBlocklyNode(root, mapTime)) {
		error = doc.SaveFile(filename.c_str());
		if (name == getCurrentDeviceName()) {
			emit currentDeviceNameChanged(name, name);
		}
	}
	return "";
}

bool DeviceManage::XMLBlocklyNode(void* pNode, QMap<QString, unsigned int> mapTime)
{
	if (!pNode) return false;
	static bool bUpdate = false;
	tinyxml2::XMLElement* element = (tinyxml2::XMLElement*)pNode;
	for (tinyxml2::XMLElement* currentele = element->FirstChildElement();
		currentele;
		currentele = currentele->NextSiblingElement()) {
		tinyxml2::XMLElement* tmpele = currentele;
		QString qstrBlock = tmpele->Attribute("type");
		if ("Fly_TimeGroup" == qstrBlock) {
			tinyxml2::XMLElement* field = tmpele->FirstChildElement("field");
			QString qstrField = field->Attribute("name");
			if (field && "GroupName" == qstrField) {
				QString qstrText = field->GetText();
				unsigned int time = mapTime.value(qstrText);
				if (0 != time) {
					bUpdate = true;
					unsigned int minute = time / 60;
					unsigned int second = time % 60;
					field = field->NextSiblingElement("field");
					QString temp = field->Attribute("name");
					if (field && "minute" == QString(field->Attribute("name"))) {
						field->SetText(minute);
					}
					field = field->NextSiblingElement("field");
					temp = field->Attribute("name");
					if (field && "second" == QString(field->Attribute("name"))) {
						field->SetText(second);
					}
				}
			}
		}
		if (!tmpele->NoChildren())
			XMLBlocklyNode(tmpele, mapTime);
	}
	return bUpdate;
}

void DeviceManage::onDeviceConrolFinished(int nCommanId, QString text, int res, QString explain)
{
	DeviceControl* pControl = dynamic_cast<DeviceControl*>(sender());
	if (!pControl) return;
	if (DeviceDataSucceed != res) {
		//失败时提示错误消息
		QString temp = pControl->getName() + text + Utility::waypointMessgeFromStatus(nCommanId, res);
		_ShowErrorMessage(temp);
	}
}