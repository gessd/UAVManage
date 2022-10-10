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
	long getX() { return ui.lineEditX->text().trimmed().toLong(); }
	void setX(long x) { ui.lineEditX->setText(QString::number(x)); }
	long getY() { return ui.lineEditY->text().trimmed().toLong(); }
	void setY(long y) { ui.lineEditY->setText(QString::number(y)); }
	void setParam(QString name, QString ip, long x, long y) {
		setName(name);
		setIp(ip);
		setX(x);
		setY(y);
	}
private:
	Ui::AddDeviceDialog ui;
};
