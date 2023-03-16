#pragma once

#include <QDialog>
#include "ui_firmwaredialog.h"

class FirmwareDialog : public QDialog
{
	Q_OBJECT

public:
	FirmwareDialog(QWidget *parent = nullptr);
	~FirmwareDialog();
	void setDeviceNameList(QStringList list);
private slots:
	void onBtnCheckFirmware();
	void onBtnManualFirmware();
private:
	Ui::FirmwareDialog ui;
};
