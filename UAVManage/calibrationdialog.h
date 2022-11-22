#pragma once

#include <QDialog>
#include "ui_calibrationdialog.h"
#include "devicecontrol.h"

class CalibrationDialog : public QDialog
{
	Q_OBJECT

public:
	CalibrationDialog(QMap<QString, DeviceControl*> map, QWidget *parent = nullptr);
	CalibrationDialog(DeviceControl* device, QWidget* parent = nullptr);
	~CalibrationDialog();
	void addLogToBrowser(QString text);
public slots:
	void onBtnCalibrationClicked();
	void onDeviceMessage(QByteArray data);
private:
	Ui::CalibrationDialogClass ui;
};
