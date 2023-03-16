#include "firmwaredialog.h"
#include <QMessageBox>

FirmwareDialog::FirmwareDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.btnCheckFirmware, &QAbstractButton::clicked, this, &FirmwareDialog::onBtnCheckFirmware);
	connect(ui.btnManualFirmware, &QAbstractButton::clicked, this, &FirmwareDialog::onBtnManualFirmware);
}

FirmwareDialog::~FirmwareDialog()
{}

void FirmwareDialog::setDeviceNameList(QStringList list)
{
	ui.listWidgetDevice->addItems(list);
}

void FirmwareDialog::onBtnCheckFirmware()
{
	//通过网络获取最新版本固件
	QMessageBox::information(this, tr("提示"), tr("功能开发中..."));
}

void FirmwareDialog::onBtnManualFirmware()
{
	//选择本地已存在固件文件
	QMessageBox::information(this, tr("提示"), tr("功能开发中..."));
}
