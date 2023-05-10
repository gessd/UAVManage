#pragma once

#include <QDialog>
#include "ui_calibrationdialog.h"
#include "devicecontrol.h"
#include <QMovie>

#define _AccIng_       "acc calib"
#define _AccFinished_  "Acc calb success"

class CalibrationDialog : public QDialog
{
	Q_OBJECT

public:
	CalibrationDialog(int calib, DeviceControl* device, QWidget* parent);
	~CalibrationDialog();
	void addLogToBrowser(QString text);
public slots:
	void onDeviceMessage(QString data);
	void onDeviceStatus(QString name, QString ip, bool connect);
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
private:
	Ui::CalibrationDialogClass ui;
	QLabel* m_pLabelBackground;
	QMovie* m_pMovieAcc;
	bool m_bAccFinished;
};
