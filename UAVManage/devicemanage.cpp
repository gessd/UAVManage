#include "devicemanage.h"
#include "qmath.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include "definesetting.h"
#include "messagelistdialog.h"

#define _ItemHeight_ 60
DeviceManage::DeviceManage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	
	//设置设备列表
	ui.listWidget->installEventFilter(this);
	connect(ui.listWidget, &QListWidget::currentItemChanged, [this](QListWidgetItem* current, QListWidgetItem* previous) {
		qDebug() << "--list currentItemChanged" << current; 
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
	m_pMenu->addAction(pActionResetName);
	m_pMenu->addAction(pActionResetIP);
	m_pMenu->addAction(pActionDicconnect);
	m_pMenu->addSeparator();
	m_pMenu->addAction(pFlyTo);
	m_pMenu->addAction(pLand);
	m_pMenu->addAction(pStop);
	//菜单响应处理
	connect(pActionResetName, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		QString qstrOldName = pControl->getName();
		QString qstrNewName = QInputDialog::getText(this, tr("重命名"), tr("请输入新名称"), QLineEdit::Normal, qstrOldName);
		if (qstrNewName.isEmpty()) return;
		if (isRepetitionName(qstrNewName)) {
			QMessageBox::warning(this, tr("提示"), tr("无法修改，名称重复"));
			return;
		}
		pControl->setName(qstrNewName);
		emit deviceRenameFinished(qstrNewName, qstrOldName);
		});
	connect(pActionResetIP, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		QString qstrNewIP = QInputDialog::getText(this, tr("IP"), tr("请输入设备IP"), QLineEdit::Normal, pControl->getIP());
		if (qstrNewIP.isEmpty()) return;
		pControl->setIp(qstrNewIP);
		emit deviceAddFinished(pControl->getName(), qstrNewIP, pControl->getX(), pControl->getY());
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
		QString qstrText = Utility::waypointMessgeFromStatus(res);
		_ShowErrorMessage(pControl->getName() + tr("起飞") + qstrText);
		});
	connect(pLand, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		int res = pControl->Fun_MAV_CMD_NAV_LAND_LOCAL(0, 0, 0, 0, 0, 0, 0);
		if (res == _DeviceStatus::DeviceDataSucceed) return;
		QString qstrText = Utility::waypointMessgeFromStatus(res);
		_ShowErrorMessage(pControl->getName() + tr("降落") + qstrText);
		});
	connect(pStop, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		int res = pControl->Fun_MAV_QUICK_STOP();
		if (res == _DeviceStatus::DeviceDataSucceed) return;
		QString qstrText = Utility::waypointMessgeFromStatus(res);
		_ShowErrorMessage(pControl->getName()+tr("急停")+qstrText);
		});
	addDevice("测试名称1", "", 0,0);
	addDevice("测试名称2", "", 0, 0);
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
}

DeviceManage::~DeviceManage()
{
}

QString DeviceManage::addDevice(QString qstrName, QString ip, float x, float y)
{
	if (qstrName.isEmpty()) {
		return tr("设备名称为空");
	}
	//判断设备名称是否重复
	if (isRepetitionName(qstrName)) {
		return tr("设备名称重复");
	}
	//TODO
	//判断设备IP是否重复
	//判断初始位置是否重复

	QListWidgetItem* item = new QListWidgetItem();
	item->setSizeHint(QSize(0, _ItemHeight_));
	ui.listWidget->addItem(item);
	//此处会耗时
	DeviceControl* pControl = new DeviceControl(qstrName, x, y, ip);
	connect(pControl, &DeviceControl::sigWaypointProcess, this, &DeviceManage::sigWaypointProcess);
	ui.listWidget->setItemWidget(item, pControl);
	ui.listWidget->setCurrentItem(item);
	emit deviceAddFinished(qstrName, ip, x, y);
	return "";
}

void DeviceManage::removeDevice()
{
	//删除选择项目，如果没有选中则删除最后一个
	int count = ui.listWidget->count();
	if (0 >= count) return;
	QListWidgetItem* item = ui.listWidget->currentItem();
	if (!item) item = ui.listWidget->item(count - 1);
	DeviceControl* m_pControl = dynamic_cast<DeviceControl*>(ui.listWidget->itemWidget(item));
	if (!m_pControl) return;
	QString name = m_pControl->getName();
	ui.listWidget->takeItem(ui.listWidget->currentRow());
	emit deviceRemoveFinished(name);
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
		qDebug() << "----选中设备" << qstrName;
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
bool DeviceManage::isRepetitionName(QString qstrName)
{
	if (0 >= ui.listWidget->count()) return false;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		if(qstrName != pDevice->getName()) continue;
		return true;
	}
	return false;
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
			qDebug() << "---- text:" << label.text();
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
		switch (comand)
		{
		case DeviceManage::_DeviceTakeoffLocal:
			pDevice->Fun_MAV_CMD_NAV_TAKEOFF_LOCAL(0, 0, 0, 0, 0, 0, _TakeoffLocalHeight_, false, true);
			break;
		case DeviceManage::_DeviceLandLocal:
			pDevice->Fun_MAV_CMD_NAV_LAND_LOCAL(0, 0, 0, 0, 0, 0, 0, false, true);
			break;
		case DeviceManage::_DeviceQuickStop:
			pDevice->Fun_MAV_QUICK_STOP(false, true);
			break;
		case DeviceManage::_DeviceSetout:
			pDevice->Fun_MAV_CMD_DO_SET_MODE(3, false);
			break;
		default:
			break;
		}
	}
}

QString DeviceManage::sendWaypoint(QString name, QVector<NavWayPointData> data)
{
	if (name.isEmpty()) return tr("设备名称错误");
	if (0 == data.count()) return tr("舞步数据为空");
	QString qstrMessage;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		QListWidgetItem* pItem = ui.listWidget->item(i);
		if (!pItem) continue;
		QWidget* pWidget = ui.listWidget->itemWidget(pItem);
		if (!pWidget) continue;
		DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
		if (!pDevice) continue;
		if (name != pDevice->getName()) continue;
		int status = pDevice->DeviceMavWaypointStart(data);
		if (_DeviceStatus::DeviceDataSucceed != status)
			return Utility::waypointMessgeFromStatus(status);
		return "";
	}
	return tr("没有可用设备");
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

DeviceControl* DeviceManage::getCurrentDevice()
{
	QListWidgetItem* item = ui.listWidget->currentItem();
	if (!item) return nullptr;
	QWidget* pWidget = ui.listWidget->itemWidget(item);
	if (!pWidget) return nullptr;
	DeviceControl* pDevice = dynamic_cast<DeviceControl*>(pWidget);
	return pDevice;
}
