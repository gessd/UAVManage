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

#define _ItemHeight_ 70
DeviceManage::DeviceManage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_pointSpace.setX(0);
	m_pointSpace.setY(0);
	m_b3DFinished = false;
	m_pFirmwareDialog = nullptr;
	m_p3dTcpSocket = nullptr;
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
	m_pMenu = new QMenu(this);
	m_pMenu->setWindowFlags(m_pMenu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	m_pMenu->setAttribute(Qt::WA_TranslucentBackground);
	QAction* pActionParam = new QAction(tr("修改"), this);
	QAction* pActionDicconnect = new QAction(tr("连接/断开"), this);
	QAction* pFlyTo = new QAction(tr("起飞"), this);
	QAction* pLand = new QAction(tr("降落"), this);
	QAction* pActionMagnetism = new QAction(tr("磁罗盘校准"), this);
	QAction* pActionAccelerometer = new QAction(tr("加计校准"), this);
	m_pActionMagnetismOpen = new QAction(tr("磁罗盘开启"), this);
	m_pActionMagnetismClose = new QAction(tr("磁罗盘关闭"), this);
	m_pActionGyro = new QAction(tr("陀螺校准"), this);
	m_pActionBaro = new QAction(tr("电调校准"), this);
	m_pActionDebug = new QAction(tr("调试"), this);
	m_pMenu->addAction(pActionParam);
	m_pMenu->addAction(pActionDicconnect);
	m_pMenu->addSeparator();
	m_pMenu->addAction(pFlyTo);
	m_pMenu->addAction(pLand);
	m_pMenu->addSeparator();
	m_pMenu->addAction(pActionMagnetism);
	m_pMenu->addAction(pActionAccelerometer);
	m_pMenu->addSeparator();
	m_pActionGyro->setProperty("Calibration", _Gyro);
	pActionMagnetism->setProperty("Calibration", _Magnetometer);
	pActionAccelerometer->setProperty("Calibration", _Accelerometer);
	m_pActionBaro->setProperty("Calibration", _Baro);
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
	connect(ui.btnBaseStation, &QAbstractButton::clicked, [this]() { 
		PlaceInfoDialog info(getSpaceSize(), this);
		info.exec();
		if (false == info.isValidStation()) return;
		QMap<QString, QPoint> map = info.getStationAddress();
		setStationAddress(map);
		});

	//设备IP地址信息
	m_pDeviceNetwork = new DeviceSerial(this);
	ui.btnSerial->setVisible(m_pDeviceNetwork->isSerialEnabled());
	connect(m_pDeviceNetwork, &DeviceSerial::sigDeviceEnabled, [this](bool enabled) { ui.btnSerial->setVisible(enabled);});
	connect(ui.btnSerial, &QAbstractButton::clicked, [this]() { m_pDeviceNetwork->exec(); });
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

QString DeviceManage::addDevice(QString qstrName, QString ip, long x, long y)
{
	if (qstrName.isEmpty()) return tr("设备名称不能为空");
	//判断设备是否重复
	QString temp = isRepetitionDevice(qstrName, ip, x, y, "111");
	if (!temp.isEmpty())  return temp;

	QListWidgetItem* item = new QListWidgetItem();
	item->setSizeHint(QSize(0, _ItemHeight_));
	ui.listWidget->addItem(item);
	//此处会耗时
	DeviceControl* pControl = new DeviceControl(qstrName, x, y, ip);
	connect(pControl, &DeviceControl::sigWaypointFinished, this, &DeviceManage::sigWaypointFinished);
	connect(pControl, &DeviceControl::sigConrolFinished, this, &DeviceManage::onDeviceConrolFinished);
	connect(pControl, &DeviceControl::sigRemoveDevice, this, &DeviceManage::onRemoveDevice);
	//需要先发送添加设备信息，用于创建默认blockly布局，当ui.listWidget->setCurrentItem触发设备切换时可以显示有布局的WEB界面
	emit deviceAddFinished(qstrName, ip, x, y);
	ui.listWidget->setItemWidget(item, pControl);
	ui.listWidget->setCurrentItem(item);
	return "";
}

void DeviceManage::clearDevice()
{
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
	if (y > (m_pointSpace.y()-100)) {
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
			if (ip == pDevice->getIP()) return tr("设备IP重复");
		}
		if ('0' != qstrRep.at(2)) {
			if (x == pDevice->getX() && y == pDevice->getY()) return tr("设备初始位置重复");
		}
	}
	return "";
}

void DeviceManage::allDeviceControl(_AllDeviceCommand comand)
{
	if (_DeviceTakeoffLocal == comand || _DevicePrepare == comand) {
		//起飞前检查所有设备状态
		//设备连接状态
		//设备电量
		//舞步已上传
		//基站校准
		//基站电量
		//无人机是否在初始位置附近
		//起飞时_DeviceTakeoffLocal检查已经准备起飞
		//TODO 需要考虑已上传舞步后又重新编辑舞步处理方式
		
		//先清空错误消息
		_MessageListClear;
		//记录出错设备名称
		QStringList listNames;
		QStringList listCheck;
		for (int i = 0; i < ui.listWidget->count(); i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if (!pItem) continue;
			QWidget* pWidget = ui.listWidget->itemWidget(pItem);
			if (!pWidget) continue;
			DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
			if (!pDevice) continue;
			if(false == pDevice->isCheckDevice()) continue;
			QString name = pDevice->getName();
			listCheck.append(name);
			listNames.append(name);
			if (false == pDevice->isConnectDevice()) {
				_ShowErrorMessage(name + tr("设备未连接无法起飞"));
				continue;
			}
#ifndef _DebugApp_
			if (pDevice->getCurrentStatus().battery < 50) {
				_ShowErrorMessage(name + tr("设备电量过低无法起飞"));
				continue;
			}
			if (0 == m_stationMap.count()) {
				_ShowErrorMessage(name + tr("基站未成功标定无法起飞"));
				continue;
			}
			if (false == pDevice->isUploadWaypoint()) {
				_ShowErrorMessage(name + tr("设备未上传舞步无法起飞"));
				continue;
			}
			if (qAbs(pDevice->getX() - pDevice->getCurrentStatus().x) > 50) {
				_ShowErrorMessage(name + tr("设备X轴方向距离初始位置超过50厘米"));
				continue;
			}
			if (qAbs(pDevice->getY() - pDevice->getCurrentStatus().y) > 50) {
				_ShowErrorMessage(name + tr("设备Y轴方向距离初始位置超过50厘米"));
				continue;
			}
#endif
			if (_DeviceTakeoffLocal == comand && false == pDevice->isPrepareTakeoff()) {
				_ShowErrorMessage(name + tr("设备未准备起飞"));
				continue;
			}
			//所有判断检查通过
			listNames.removeOne(name);
		}
		if (false == listNames.isEmpty()) {
			QString qstrError = listNames.join("、");
			QString error = tr("检查出错，请重试") + qstrError;
			_ShowErrorMessage(error);
			QMessageBox::warning(this, tr("警告"), error);
			return;
		}
		if (listCheck.isEmpty()) {
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
	int x = 0;
	int y = 0;
	QStringList listCheck;
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
		case _DevicePrepare:
			res = pDevice->Fun_MAV_CMD_DO_SET_MODE(3, false);
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
			int nInterval = 50;			//无人机间隔
			int c = getSpaceSize().x() / nInterval;
			if (i > 0) {
				int r = i % c;
				x = r * nInterval;
				if (0 == r) y += nInterval;
			}
			res = pDevice->Fun_MAV_Defined_Regain(x, y);
			qstrText = tr("回收");
			break;
		}
		default:
			break;
		}
		qDebug() << QString("%1%2错误码%3").arg(qstrName).arg(qstrText).arg(res);
		if (_DeviceStatus::DeviceDataSucceed != res) {
			_ShowErrorMessage(qstrName+qstrText+tr("出错:")+ Utility::waypointMessgeFromStatus(comand, res));
		}
	}
	if (listCheck.isEmpty()) {
		_ShowInfoMessage(tr("未选中无人机"));
		return;
	}
	if (_DeviceTakeoffLocal == comand) emit sigTakeoffFinished(true);
	else emit sigTakeoffFinished(false);
}

QString DeviceManage::waypointComposeAndUpload(QString qstrProjectFile, bool upload)
{
	_MessageListClear
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
		if(false == pDevice->isCheckDevice()) continue;
#ifndef _DebugApp_
		if (upload) {
			if (false == pDevice->isConnectDevice()) {
				_ShowErrorMessage(name + tr("设备没有连接无法上传舞步"));
				continue;
			}
			if (qAbs(pDevice->getX() - pDevice->getCurrentStatus().x) > 50) {
				_ShowErrorMessage(name + tr("设备X轴方向距离初始位置超过50厘米，无法上传舞步"));
				continue;
			}
			if (qAbs(pDevice->getY() - pDevice->getCurrentStatus().y) > 50) {
				_ShowErrorMessage(name + tr("设备Y轴方向距离初始位置超过50厘米，无法上传舞步"));
				continue;
			}
		}
#endif
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
		//执行python脚本之前初始参数，用于检查无人机编程参数
		pythonThread.initParam(m_pointSpace.x(), m_pointSpace.y(), pDevice->getName(), pDevice->getX(), pDevice->getY());
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
			_ShowErrorMessage(name + tr("舞步转换失败")+pythonThread.getErrorString(state));
			qstrErrorNames.append("," + pDevice->getName());
			continue;
		}
		QVector<NavWayPointData> data = pythonThread.getWaypointData();
		//最少2条，第一条是设置的起始位置
		if (data.count() <= 1) {
			_ShowErrorMessage(name + tr("没有舞步信息"));
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
		if (false == upload) {
			//清空三维仿真碰撞信息
			reset3DStatus();
			_ShowInfoMessage(name + tr("生成舞步完成"));
		}
		else {
#ifndef _DebugApp_
			//上次舞步到飞控之前检查是否进行三维仿真
			qInfo() << QString("三维仿真是否完成:%1").arg(m_b3DFinished);
			if (false == m_b3DFinished) {
				_ShowErrorMessage("三维仿真未完成，无法上传舞步到无人机");
				QMessageBox::warning(this, "警告", "三维仿真未完成，无法上传舞步到无人机");
				return false;
			}
			qInfo() << "三维仿真中碰撞设备" << m_map3DCollision;
			if (false == m_map3DCollision.isEmpty()) {
				QString text;
				QStringList list = m_map3DCollision.keys();
				foreach(QString name, list) {
					text.append(name + "，");
				}
				text.append("舞步存在碰撞风险，请检查后重试");
				_ShowErrorMessage(text);
				QMessageBox::warning(this, "警告", text);
				return false;
			}
#endif
			//上传航点到飞控
			qInfo() << "准备上次舞步到飞控";
			int status = pDevice->DeviceMavWaypointStart(data);
			if (_DeviceStatus::DeviceDataSucceed != status) {
				qstrErrorNames.append("," + pDevice->getName());
				_ShowErrorMessage(name + "上传舞步失败"+ Utility::waypointMessgeFromStatus(_DeviceWaypoint, status));
			}
		}
		map.insert(name, data);
	}
	//发送航点到三维
	sendWaypointTo3D(map);
	return qstrErrorNames;
}

void DeviceManage::setUpdateWaypointTime(int second)
{

}

void DeviceManage::setCurrentPlayeState(qint8 state)
{

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
	//QJsonObject obj3dmsg;
	//obj3dmsg.insert(_Ver_, _VerNum_);
	//obj3dmsg.insert(_Tag_, _TabName_);
	//obj3dmsg.insert(_ID_, _3dDeviceMusicPath);
	//obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	////obj3dmsg.insert(_Data_, filePath);
	//QJsonObject data;
	//data.insert("file", filePath);
	//data.insert("image", qstrPixmapPath);
	//obj3dmsg.insert(_Data_, data);
	//sendMessageTo3D(obj3dmsg);
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
			} else if (_WaypointHover == waypoint.commandID) {
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
				//三维中需要第0秒为初始位置
				waypoint.commandID = _WaypointFly;
				waypoint.param1 = waypoint.param2 = waypoint.param3 = waypoint.param4 = 0;
				red = green = 0;
				blue = 255;
				status = 8;	//开始时点亮LED
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
				//降落速度每秒0.5米，计算降落时间
				status = 9; //降落熄灭LED
				red = green = blue = 0;
				int x = waypoint.x;
				int y = waypoint.y;
				int z = waypoint.z;
				int d = getDistance(lastX, lastY, lastZ, x, y, z);
				waypoint.param3 = d  / 50;
				waypoint.commandID = _WaypointFly;
			}

			if(_WaypointFly != waypoint.commandID) continue;
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
#ifdef _WaypointUseTime_
			time = waypoint.param3 * 1000;
#else
			if (waypoint.param1 > 0) {
				time = time + waypoint.param1 * 1000;
			}
#endif
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
	m_map3DCollision.clear();
}

bool DeviceManage::eventFilter(QObject* watched, QEvent* event)
{
	if (!isEnabled()) return false;
	if (ui.listWidget == watched) {
		//设备列表菜单
		if (QEvent::ContextMenu != event->type()) return false;
		if (!ui.listWidget->itemAt(ui.listWidget->mapFromGlobal(QCursor::pos()))) return false;
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
		if(!pDevice->isConnectDevice()) continue;
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
		if(temp != name) continue;
		ui.listWidget->takeItem(i);
		emit deviceRemoveFinished(name);
		return;
	}
	
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
		if(pDevice->getName() != name) continue;
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
	int id = jsonObj.value(_ID_).toInt();
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
		//三维仿真中发生无人机碰撞
		qDebug() << "三维仿真中发生无人机碰撞" << jsonObj;
		QString name = jsonObj.value("name").toString();
		QStringList list = getDeviceNameList();
		if (list.contains(name)) {
			m_map3DCollision.insert(name, true);
		}
	}
	else if (_3dDeviceFinished == id) {
		qDebug() << "三维仿真结束" << jsonObj;
		m_b3DFinished = true;
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
	//TODO 测试使用，暂时去掉提示框
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
		return "内部错误，无法更新动作组时间";
	}
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return "内部错误，无法更新动作组时间";
	if (XMLBlocklyNode(root, mapTime)) {
		error = doc.SaveFile(filename.c_str());
		emit currentDeviceNameChanged(name, name);
	}
	return "";
}

bool DeviceManage::XMLBlocklyNode(void* pNode, QMap<QString, unsigned int> mapTime)
{
	if (!pNode) return false;
	static bool bUpdate = false;
	tinyxml2::XMLElement* element = (tinyxml2::XMLElement*)pNode;
	for (tinyxml2::XMLElement* currentele = element->FirstChildElement(); currentele; currentele = currentele->NextSiblingElement())
	{
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

void DeviceManage::onDeviceConrolFinished(QString text, int res, QString explain)
{
	if (DeviceDataSucceed == res) return;
	DeviceControl* pControl = dynamic_cast<DeviceControl*>(sender());
	if (!pControl) return;
	text.prepend(pControl->getName());
	_ShowErrorMessage(text + tr("错误:") + Utility::waypointMessgeFromStatus(_DeviceWaypoint, res));
}