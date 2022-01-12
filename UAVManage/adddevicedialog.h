#pragma once

#include <QDialog>
#include "ui_adddevicedialog.h"

class AddDeviceDialog : public QDialog
{
	Q_OBJECT

public:
	AddDeviceDialog(QString qstrName, QWidget *parent = Q_NULLPTR);
	~AddDeviceDialog();
	void setName(QString qstrName) { ui.lineEditName->setText(qstrName); }
	QString getName() { return ui.lineEditName->text().trimmed(); }
	QString getIP() { return ui.lineEditIP->text().trimmed(); }
	float getX() { return ui.lineEditX->text().trimmed().toFloat(); }
	float getY() { return ui.lineEditY->text().trimmed().toFloat(); }
private:
	Ui::AddDeviceDialog ui;
};
