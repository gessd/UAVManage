#pragma once

#include <QDialog>
#include "ui_calibrationdialog.h"
#include "devicecontrol.h"

class CalibrationDialog : public QDialog
{
	Q_OBJECT

public:
	CalibrationDialog(QMap<QString, DeviceControl*> map, QWidget *parent = nullptr);
	~CalibrationDialog();
private:
	Ui::CalibrationDialogClass ui;
};
