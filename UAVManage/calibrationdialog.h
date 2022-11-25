#pragma once

#include <QDialog>
#include "ui_calibrationdialog.h"
#include "devicecontrol.h"

class CalibrationDialog : public QDialog
{
	Q_OBJECT

public:
	CalibrationDialog(QMap<QString, DeviceControl*> map, QWidget *parent);
	CalibrationDialog(DeviceControl* device, QWidget* parent);
	~CalibrationDialog();
	void addLogToBrowser(QString text);
public slots:
	void onBtnCalibrationClicked();
	void onDeviceMessage(QByteArray data);
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
private:
	Ui::CalibrationDialogClass ui;
	QLabel* m_pLabelBackground;
};
