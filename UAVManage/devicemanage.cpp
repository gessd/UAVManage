#include "devicemanage.h"
#include "qmath.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include "definesetting.h"

#define _ItemHeight_ 60
DeviceManage::DeviceManage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	
	//设置设备列表
	ui.listWidget->installEventFilter(this);
	connect(ui.listWidget, &QListWidget::itemPressed, [this](QListWidgetItem* item) {qDebug() << "--list itemPressed"; });
	connect(ui.listWidget, &QListWidget::itemClicked, [this](QListWidgetItem* item) {qDebug() << "--list itemClicked"; });
	connect(ui.listWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem* item) {qDebug() << "--list itemDoubleClicked"; });
	connect(ui.listWidget, &QListWidget::itemActivated, [this](QListWidgetItem* item) {qDebug() << "--list itemActivated"; });
	connect(ui.listWidget, &QListWidget::itemEntered, [this](QListWidgetItem* item) {qDebug() << "--list itemEntered"; });
	connect(ui.listWidget, &QListWidget::itemChanged, [this](QListWidgetItem* item) {qDebug() << "--list itemChanged"; });
	connect(ui.listWidget, &QListWidget::currentItemChanged, [this](QListWidgetItem* current, QListWidgetItem* previous) {
		qDebug() << "--list currentItemChanged" << current; 
		QString name;
		QString previousname;
		if (current) {
			DeviceControl* pControl = dynamic_cast<DeviceControl*>(current);
			if (pControl) name = pControl->getName();
		}
		if (current) {
			DeviceControl* pControl = dynamic_cast<DeviceControl*>(previous);
			if (pControl) previousname = pControl->getName();
		}
		emit currentDeviceNameChanged(name, previousname);
		});
	connect(ui.listWidget, &QListWidget::currentTextChanged, [this](const QString& currentText) {qDebug() << "--list currentTextChanged" << currentText; });
	connect(ui.listWidget, &QListWidget::currentRowChanged, [this](int currentRow) {qDebug() << "--list currentRowChanged" << currentRow; });
	connect(ui.listWidget, &QListWidget::itemSelectionChanged, [this]() {qDebug() << "--list itemSelectionChanged"; });

	//添加右键菜单
	m_pMenu = new QMenu(this);
	QAction* pActionResetName = new QAction(tr("重命名"), this);
	QAction* pActionResetIP = new QAction(tr("修改IP"), this);
	QAction* pActionDicconnect = new QAction(tr("连接/断开"), this);
	QAction* pFlyTo = new QAction(tr("起飞"), this);
	m_pMenu->addAction(pActionResetName);
	m_pMenu->addAction(pActionResetIP);
	m_pMenu->addAction(pActionDicconnect);
	m_pMenu->addAction(pFlyTo);
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
		});
	connect(pActionResetIP, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		QString qstrNewIP = QInputDialog::getText(this, tr("IP"), tr("请输入设备IP"), QLineEdit::Normal, pControl->getIP());
		if (qstrNewIP.isEmpty()) return;
		pControl->setIp(qstrNewIP);
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
		int nTest = pControl->Fun_MAV_CMD_NAV_TAKEOFF_LOCAL(1, 2, 3, 4, 5, 6, 7);
		});

	connect(ui.btnAddDevice, &QAbstractButton::clicked, [this]() {
		AddDeviceDialog dialog(getNewDefaultName(), this);
		if (QDialog::Accepted != dialog.exec())return;
		QString qstrError = addDevice(dialog.getName(), dialog.getIP(), dialog.getX(), dialog.getY());
		if (!qstrError.isEmpty()) {
			QMessageBox::warning(this, tr("错误"), qstrError);
		}
		});
	connect(ui.btnRemoveDevice, &QAbstractButton::clicked, [this]() { removeDevice(); });


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
	ui.listWidget->setItemWidget(item, pControl);
	ui.listWidget->setCurrentItem(item);
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
	ui.listWidget->takeItem(ui.listWidget->currentRow());
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

bool DeviceManage::eventFilter(QObject* watched, QEvent* event)
{
	if (ui.listWidget == watched) {
		//设备列表菜单
		if (QEvent::ContextMenu != event->type()) return false;
		if (!ui.listWidget->itemAt(mapFromGlobal(QCursor::pos()))) return false;
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
