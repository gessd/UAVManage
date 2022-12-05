#pragma once

#include <QDialog>
#include "ui_calibrationdialog.h"
#include "devicecontrol.h"

#define _AccIng_   "acc calib"

class CalibrationDialog : public QDialog
{
	Q_OBJECT

public:
	CalibrationDialog(QMap<QString, DeviceControl*> map, QWidget *parent);
	CalibrationDialog(int calib, DeviceControl* device, QWidget* parent);
	~CalibrationDialog();
	void addLogToBrowser(QString text);
public slots:
	void onBtnCalibrationClicked();
	void onDeviceMessage(QString data);
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
private:
	Ui::CalibrationDialogClass ui;
	QLabel* m_pLabelBackground;
};
