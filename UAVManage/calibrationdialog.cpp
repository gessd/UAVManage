#include "calibrationdialog.h"
#include <QDateTime>
#include <QDebug>

CalibrationDialog::CalibrationDialog(QMap<QString, DeviceControl*> map, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	connect(ui.btnMagnetism, &QAbstractButton::clicked, this, &CalibrationDialog::onBtnCalibrationClicked);
	connect(ui.btnGyro, &QAbstractButton::clicked, this, &CalibrationDialog::onBtnCalibrationClicked);
	connect(ui.btnAccelerometer, &QAbstractButton::clicked, this, &CalibrationDialog::onBtnCalibrationClicked);
	connect(ui.btnBaro, &QAbstractButton::clicked, this, &CalibrationDialog::onBtnCalibrationClicked);
	QStringList keys = map.keys();
	foreach(QString name, keys) {
		QTreeWidgetItem* pItem = new QTreeWidgetItem;
		pItem->setCheckState(0, Qt::Unchecked);
		pItem->setText(0, name);
		pItem->setData(0, Qt::UserRole, QVariant::fromValue(map.value(name)));
		ui.treeWidget->addTopLevelItem(pItem);
	}
}

CalibrationDialog::CalibrationDialog(DeviceControl* device, QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	ui.treeWidget->setVisible(false);
	ui.widget->setVisible(false);
	connect(device, &DeviceControl::sigLogMessage, this, &CalibrationDialog::onDeviceMessage);
}

CalibrationDialog::~CalibrationDialog()
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
}

void CalibrationDialog::addLogToBrowser(QString text)
{
	qDebug() << text;
	ui.textBrowser->append(QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]:"));
	ui.textBrowser->append(text);
}

void CalibrationDialog::onBtnCalibrationClicked()
{
	QAbstractButton* pButton = dynamic_cast<QAbstractButton*>(sender());
	if (nullptr == pButton) return;
	int count = ui.treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++) {
		QTreeWidgetItem* pItem = ui.treeWidget->topLevelItem(i);
		if(nullptr == pItem) continue;
		if (Qt::Unchecked == pItem->checkState(0)) continue;
		DeviceControl* pDevice = pItem->data(0, Qt::UserRole).value<DeviceControl*>();
		if(nullptr == pDevice) continue;
		if (false == pDevice->isConnectDevice()) {
			addLogToBrowser(pDevice->getName()+ tr("设备未连接，无法校准"));
			continue;
		}
		addLogToBrowser(pDevice->getName() + tr("开始校准"));
		if (ui.btnGyro == pButton) {
			pDevice->Fun_MAV_CALIBRATION(1, 0, 0, 0, 0, 0, 0);
		}
		else if (ui.btnMagnetism == pButton) {
			pDevice->Fun_MAV_CALIBRATION(0, 1, 1, 0, 0, 0, 0);
		}
		else if (ui.btnAccelerometer == pButton) {
			pDevice->Fun_MAV_CALIBRATION(0, 0, 0, 0, 1, 0, 0);
		}
		else if (ui.btnBaro == pButton) {
			pDevice->Fun_MAV_CALIBRATION(0, 0, 0, 0, 0, 0, 1);
		}
	}
}

void CalibrationDialog::onDeviceMessage(QString data)
{
	addLogToBrowser(data);
}

void CalibrationDialog::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->show();
}

void CalibrationDialog::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}
