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
	void setIp(QString ip) { ui.lineEditIP->setText(ip); };
	float getX() { return ui.lineEditX->text().trimmed().toFloat(); }
	void setX(float x) { ui.lineEditX->setText(QString::number(x, 'f', 2)); }
	float getY() { return ui.lineEditY->text().trimmed().toFloat(); }
	void setY(float y) { ui.lineEditY->setText(QString::number(y, 'f', 2)); }
	void setParam(QString name, QString ip, float x, float y) {
		setName(name);
		setIp(ip);
		setX(x);
		setY(y);
	}
private:
	Ui::AddDeviceDialog ui;
};
