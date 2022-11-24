#pragma once

#include <QDialog>
#include "ui_aboutdialog.h"

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	AboutDialog(QWidget *parent = nullptr);
	~AboutDialog();
private:
	Ui::AboutDialogClass ui;
};
