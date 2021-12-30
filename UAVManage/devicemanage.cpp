#include "devicemanage.h"
#include "qmath.h"
#include <QMessageBox>
#include <QInputDialog>

#define _DeviceNamePrefix_ "无人机"
#define _ItemHeight_ 60
DeviceManage::DeviceManage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	
	//右键菜单
	m_pMenu = new QMenu(this);
	QAction* pActionResetName = new QAction(tr("重命名"), this);
	QAction* pActionResetIP = new QAction(tr("修改IP"), this);
	m_pMenu->addAction(pActionResetName);
	m_pMenu->addAction(pActionResetIP);
	ui.listWidget->installEventFilter(this);
	connect(pActionResetName, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		QString qstrOldName = pControl->getName();
		QString qstrNewName = QInputDialog::getText(this, tr("重命名"), tr("请输入新名称"));
		if (qstrNewName.isEmpty()) return;
		if (isRepetitionName(qstrNewName)) {
			QMessageBox::warning(this, tr("提示"), tr("无法修改，名称重复"));
			return;
		}
		
		for (int i = 0; i < m_listDeviceNames.count(); i++) {
			if (m_listDeviceNames.at(i) != qstrOldName) continue;
			m_listDeviceNames[i] = qstrNewName;
			break;
		}
		pControl->setName(qstrNewName);
		});
	connect(pActionResetIP, &QAction::triggered, [this](bool checked) {
		DeviceControl* pControl = getCurrentDevice();
		if (!pControl) return;
		QString qstrNewIP = QInputDialog::getText(this, tr("IP"), tr("请输入设备IP"));
		if (qstrNewIP.isEmpty()) return;
		pControl->setIp(qstrNewIP);
		});

	connect(ui.btnAddDevice, &QAbstractButton::clicked, [this]() {
		AddDeviceDialog dialog(getNewDefaultName(), this);
		if (QDialog::Accepted != dialog.exec())return;
		addDevice(dialog.getName(), dialog.getIP());
		});
	connect(ui.btnRemoveDevice, &QAbstractButton::clicked, [this]() { removeDevice(); });
}

DeviceManage::~DeviceManage()
{
}

bool DeviceManage::addDevice(QString qstrName, QString ip)
{
	if (qstrName.isEmpty()) qstrName = getNewDefaultName();
	if (isRepetitionName(qstrName)) {
		QMessageBox::warning(this, tr("提示"), tr("无法添加设备,名称重复"));
		return false;
	}
	m_listDeviceNames.append(qstrName);
	QListWidgetItem* item = new QListWidgetItem();
	item->setSizeHint(QSize(0, _ItemHeight_));
	ui.listWidget->addItem(item);
	ui.listWidget->setItemWidget(item, new DeviceControl(qstrName));
	ui.listWidget->setCurrentItem(item);
	return true;
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
	//删除设备需要删除已记录的设备名称
	m_listDeviceNames.removeOne(name);
	ui.listWidget->takeItem(ui.listWidget->currentRow());
}

QString DeviceManage::getNewDefaultName()
{
	int index = 0;
	foreach(QString name, m_listDeviceNames){
		QStringList list = name.split(_DeviceNamePrefix_);
		if (list.count() < 2) continue;
		int n = list.at(1).toInt();
		index = qMax(index, n);
	}
	return QString("%1%2").arg(_DeviceNamePrefix_).arg(index+1);
}

bool DeviceManage::isRepetitionName(QString qstrName)
{
	if (m_listDeviceNames.isEmpty()) return false;
	return m_listDeviceNames.contains(qstrName);
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
