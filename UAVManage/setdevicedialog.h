#pragma once

#include <QDialog>
#include "ui_setdevicedialog.h"

class SetDeviceDialog : public QDialog
{
	Q_OBJECT

public:
	SetDeviceDialog(QWidget *parent = Q_NULLPTR);
	~SetDeviceDialog();

private:
	Ui::SetDeviceDialog ui;
};
