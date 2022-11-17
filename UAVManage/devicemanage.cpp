#include "devicemanage.h"
#include "qmath.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include "definesetting.h"
#include "messagelistdialog.h"
#include "devicedebug.h"
#include "define3d.h"

#define _ItemHeight_ 80
DeviceManage::DeviceManage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_pointSpace.setX(0);
	m_pointSpace.setY(0);
	m_p3dTcpSocket = nullptr;
	m_p3dTcpServer = new QTcpServer(this);
	m_p3dTcpServer->listen(QHostAddress::Any, _TcpPort_);
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
	m_pMenu = new QMenu(this);
	m_pMenu->setWindowFlags(m_pMenu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	m_pMenu->setAttribute(Qt::WA_TranslucentBackground);
	QAction* pActionParam = new QAction(tr("修改"), this);
	QAction* pActionResetName = new QAction(tr("重命名"), this);
	QAction* pActionResetIP = new QAction(tr("修改IP"), this);
	QAction* pActionDicconnect = new QAction(tr("连接/断开"), this);
	QAction* pFlyTo = new QAction(tr("起飞"), this);
	QAction* pLand = new QAction(tr("降落"), this);
	QAction* pStop = new QAction(tr("急停"), this);
	QAction* pDebug = new QAction(tr("调试"), this);
	m_pMenu->addAction(pActionParam);
	//m_pMenu->addAction(pActionResetName);
	//m_pMenu->addAction(pActionResetIP);
	m_pMenu->addAction(pActionDicconnect);
	m_pMenu->addSeparator();
	m_pMenu->addAction(pFlyTo);
	m_pMenu->addAction(pLand);
	m_pMenu->addAction(pStop);
	m_pMenu->addSeparator();
	m_pMenu->addAction(pDebug);
	//菜单响应处理
	connect(pActionParam, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		QString qstrName = pControl->getName();
		AddDeviceDialog dialog(qstrName, this);
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
			if (m_p3dTcpSocket) {
				QJsonObject obj3dmsg;
				obj3dmsg.insert(_Ver_, _VerNum_);
				obj3dmsg.insert(_Tag_, _TabName_);
				obj3dmsg.insert(_ID_, _3dDeviceRename);
				obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
				QJsonObject data;
				data.insert("oldName", qstrName);
				data.insert("newName", qstrNewName);
				obj3dmsg.insert(_Data_, data);
				sendMessageTo3D(obj3dmsg);
			}
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
	connect(pActionResetName, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		QString qstrOldName = pControl->getName();
		QString qstrNewName = QInputDialog::getText(this, tr("重命名"), tr("请输入新名称"), QLineEdit::Normal, qstrOldName);
		if (qstrNewName.isEmpty()) return;
		if (qstrOldName == qstrNewName) return;
		if (!isRepetitionDevice(qstrNewName, "",0,0, "100").isEmpty()) {
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
		if (!isRepetitionDevice("", qstrNewIP, 0, 0, "010").isEmpty()) {
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
	connect(ui.btnAddDevice, &QAbstractButton::clicked, [this]() {
		AddDeviceDialog dialog(getNewDefaultName(), this);
		if (QDialog::Accepted != dialog.exec())return;
		QString qstrError = addDevice(dialog.getName(), dialog.getIP(), dialog.getX(), dialog.getY());
		if (!qstrError.isEmpty()) {
			QMessageBox::warning(this, tr("错误"), qstrError);
		}
		});
	connect(ui.btnFlyTakeoff, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceTakeoffLocal); });
	connect(ui.btnFlyLand, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceLandLocal); });
	connect(ui.btnFlyStop, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceQuickStop); });
	connect(ui.btnQueue, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceQueue); });
	connect(ui.btnRegain, &QAbstractButton::clicked, [this]() { allDeviceControl(_DeviceRegain); });

	ui.checkBoxAutoLand->setVisible(false);
	ui.checkBoxMagnetismStatus->setVisible(false);

	connect(&m_timerUpdateStatus, &QTimer::timeout, this, &DeviceManage::onUpdateStatusTo3D);
	connect(&m_timerMessage3D, &QTimer::timeout, this, &DeviceManage::onTimeout3DMessage);
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
	connect(pControl, &DeviceControl::sigWaypointProcess, this, &DeviceManage::sigWaypointProcess);
	connect(pControl, &DeviceControl::sigConrolFinished, this, &DeviceManage::onDeviceConrolFinished);
	connect(pControl, &DeviceControl::sigRemoveDevice, this, &DeviceManage::onRemoveDevice);
	//需要先发送添加设备信息，用于创建默认blockly布局，当ui.listWidget->setCurrentItem触发设备切换时可以显示有布局的WEB界面
	emit deviceAddFinished(qstrName, ip, x, y);
	ui.listWidget->setItemWidget(item, pControl);
	ui.listWidget->setCurrentItem(item);
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
	if (DeviceManage::_DeviceTakeoffLocal == comand) {
		//起飞前增加倒计时
		QLabel label(dynamic_cast<QWidget*>(parent()));
		//设置窗体的背景色,这里的百分比就是透明度
		label.setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);color:#FF0000;border:1px solid blue;font-size:100px;"));
		//label.setGeometry(dynamic_cast<QWidget*>(parent())->rect()); //获取父窗体的几何形状设置当前窗口
		label.setFixedSize(dynamic_cast<QWidget*>(parent())->size());
		label.show();
		label.setAlignment(Qt::AlignCenter);
		QFont font = label.font();
		font.setPixelSize(font.pixelSize()*4);
		//label.setFont(font);
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
	int x = 0;
	int y = 0;
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
		case DeviceManage::_DeviceQueue:
			//降落到初始位置
			pDevice->Fun_MAV_Defined_Queue(pDevice->getX(), pDevice->getY());
			qstrText = tr("列队");
			break;
		case DeviceManage::_DeviceRegain:
		{
			//按顺序排列
			int nInterval = 50;			//无人机间隔
			int c = getSpaceSize().x() / nInterval;
			if (i > 0) {
				int r = i % c;
				x = r * nInterval;
				if (0 == r) y += nInterval;
			}
			pDevice->Fun_MAV_Defined_Regain(x, y);
			qstrText = tr("回收");
			break;
		}
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

void DeviceManage::waypointComposeAndUpload(QString qstrProjectFile, bool upload)
{
	_MessageListClear
	//MAP用于统一发送航点信息到三维
	QMap<QString, QVector<NavWayPointData>> map;
	qDebug() << "准备生成舞步信息";
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		QString name = pDevice->getName();
		if (name.isEmpty()) continue;
		QFileInfo infoProject(qstrProjectFile);
		QString qstrDevicePyFile = infoProject.path() + _ProjectDirName_ + name + _PyFileSuffix_;
		if (false == QFile::exists(qstrDevicePyFile)) continue;
		QFile file(qstrDevicePyFile);
		if (!file.open(QIODevice::ReadOnly)) continue;
		QByteArray arrData = file.readAll();
		file.close();
		if (arrData.isEmpty()) {
			_ShowErrorMessage(name + tr("没有编写舞步"));
			continue;
		}
		//执行python脚本之前初始参数，用于检查无人机编程参数
		pythonThread.initParam(m_pointSpace.x(), m_pointSpace.y(), pDevice->getName(), pDevice->getX(), pDevice->getY());
		//生成舞步过程必须一个个生成，python交互函数是静态全局，所以同时只能执行一个设备生成舞步
		if (!pythonThread.compilePythonCode(arrData)) {
			//生成舞步失败
			_ShowErrorMessage(name + tr("解析舞步积木块失败"));
			continue;
		}
		//TODO 等待python文件执行完成，此处需要优化，有可能造成死循环
		while (!pythonThread.isFinished()) {
			QApplication::processEvents();
		}
		if (PythonSuccessful != pythonThread.getLastState()) {
			_ShowErrorMessage(name + tr("舞步转换失败"));
			continue;
		}
		QVector<NavWayPointData> data = pythonThread.getWaypointData();
		//最少1条，第一条是设置的起始位置
		if (1 > data.count()) {
			_ShowWarningMessage(name + tr("没有舞步信息"));
			continue;
		}
		qDebug() << name << "航点数据" << data.count();
		//TODO 判断最低起飞高度 暂定为0
		if (0 >= data.at(1).z) {
			_ShowErrorMessage(name + tr("起飞高度太低"));
			continue;
		}
		//保存航点数据到本地，便于查看
		QFile fileSvg(QApplication::applicationDirPath() + "/waypoint/" + name + ".csv");
		if (fileSvg.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			QTextStream text_stream(&fileSvg);
			text_stream.setCodec("utf-8");
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
			_ShowInfoMessage(name + tr("生成舞步完成"));
		}
		else {
			//上传航点到飞控
			int status = pDevice->DeviceMavWaypointStart(data);
			if (_DeviceStatus::DeviceDataSucceed != status) _ShowWarningMessage(name+Utility::waypointMessgeFromStatus(status));
		}
		map.insert(name, data);
	}
	//发送航点到三维
	sendWaypointTo3D(map);
}

void DeviceManage::setUpdateWaypointTime(int second)
{
	return;
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

void DeviceManage::setCurrentMusicPath(QString filePath, QPixmap pixmap)
{
	if (!QFile::exists(filePath)) return;
	QFileInfo info(filePath);
	QString qstrPixmapPath = info.path() + "/music.png";
	if (false == pixmap.save(qstrPixmapPath)) {
		qDebug() << "保持音乐波形图片失败";
		return;
	}
	QJsonObject obj3dmsg;
	obj3dmsg.insert(_Ver_, _VerNum_);
	obj3dmsg.insert(_Tag_, _TabName_);
	obj3dmsg.insert(_ID_, _3dDeviceMusicPath);
	obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	//obj3dmsg.insert(_Data_, filePath);
	QJsonObject data;
	data.insert("file", filePath);
	data.insert("image", qstrPixmapPath);
	obj3dmsg.insert(_Data_, data);
	sendMessageTo3D(obj3dmsg);
}

void DeviceManage::sendWaypointTo3D(QMap<QString, QVector<NavWayPointData>> map)
{
	qDebug() << "发送航点列表到三维界面";
	if (!m_p3dTcpSocket) return;
	QStringList listNmae = map.keys();
	QJsonObject obj3dmsg;
	obj3dmsg.insert(_Ver_, _VerNum_);
	obj3dmsg.insert(_Tag_, _TabName_);
	obj3dmsg.insert(_ID_, _3dDeviceWaypoint);
	obj3dmsg.insert(_Time_, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	QJsonArray jsonData;

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

		for (int i = 0; i < data.count(); i++) {
			NavWayPointData waypoint = data.at(i);
			if (_WaypointSpeed == waypoint.commandID) {
				//设置飞行速度
				speed = waypoint.param1;
				continue;
				if (speed <= 0) speed = 60;
			}
			if (_WaypointHover == waypoint.commandID) {
				waypoint.x = lastX;
				waypoint.y = lastY;
				waypoint.z = lastZ;
				waypoint.commandID = _WaypointFly;
			}
			//TODO 非航点信息暂时不处理
			if(_WaypointFly != waypoint.commandID) continue;
			int x = waypoint.x;
			int y = waypoint.y;
			int z = waypoint.z;
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
			int time = d / speed * 1000;
			if (waypoint.param1 > 0) {
				time = time + waypoint.param1 * 1000;
			}
			timesum += time;
			QList<QVariant> value;
			value << timesum << 20 << 0 << 0 << x << y << z << 16;
			arrWaypoint.append(QJsonArray::fromVariantList(value));
			lastX = x;
			lastY = y;
			lastZ = z;
		}
		objDevice.insert("list", arrWaypoint);
		jsonData.append(objDevice);
	}
	obj3dmsg.insert(_Data_, jsonData);
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
	qDebug() << "三维窗口新连接" << m_p3dTcpSocket;
	connect(m_p3dTcpSocket, &QTcpSocket::readyRead, [this]() {
		QByteArray arrData = m_p3dTcpSocket->readAll();
		qDebug() << "收到3D消息:" << arrData;
		//解析三维消息并把发送记录中的消息删除
		QList<QByteArray> list = arrData.split(_MsgTail);
		foreach (QByteArray data, list){
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
	if (!m_timerMessage3D.isActive()) m_timerMessage3D.start(10 * 1000);
	//发送无人机设备列表
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
	QStringList keys = m_stationMap.keys();
	foreach(QString name, keys) {
		QJsonObject obj;
		obj.insert("name", name);
		obj.insert("x", m_stationMap.value(name).x());
		obj.insert("y", m_stationMap.value(name).y());
		arrStation.append(obj);
	}
	obj3dmsg.insert("station", arrStation);
	sendMessageTo3D(obj3dmsg);
	emit sig3DDialogStatus(true);
	//实时状态定时放到最后，必须在发送设备列表之后才能发送实时位置数据
	if (!m_timerUpdateStatus.isActive()) m_timerUpdateStatus.start(1000);
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
		////TODO 三维测试用，便于更新无人机位置
		//if (0 == i) {
		//	int z = QDateTime::currentDateTime().toString("ss").toInt();
		//	int x = z / 10;
		//	int y = z;
		//	device.insert("x", x);
		//	device.insert("y", y);
		//	device.insert("z", z);
		//}
		jsonArr.append(device);
	}
	if (0 == jsonArr.count()) return;
	obj3dmsg.insert(_Data_, jsonArr);
	sendMessageTo3D(obj3dmsg);
}

void DeviceManage::onRemoveDevice(QString name)
{
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
		return;
	}
	
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
		m_map3DMsgRecord.insert(id, json3d);
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
	QJsonParseError jsonError;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &jsonError);
	if (QJsonParseError::NoError != jsonError.error || jsonDoc.isEmpty() || !jsonDoc.isObject()) {
		return;
	}
	QJsonObject jsonObj = jsonDoc.object();
	int id = jsonObj.value(_ID_).toInt();
	if (m_map3DMsgRecord.contains(id)) {
		m_map3DMsgRecord.remove(id);
	}
}

int DeviceManage::getDistance(int x1, int y1, int z1, int x2, int y2, int z2)
{
	int d = qSqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2));
	return d;
}

void DeviceManage::onDeviceConrolFinished(QString text, int res, QString explain)
{
	if (DeviceDataSucceed == res) return;
	DeviceControl* pControl = dynamic_cast<DeviceControl*>(sender());
	if (!pControl) return;
	text.prepend(pControl->getName());
	_ShowErrorMessage(text + tr("错误:") + Utility::waypointMessgeFromStatus(res));
}