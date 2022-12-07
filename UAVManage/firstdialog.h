#pragma once

#include <QDialog>
#include "ui_firstdialog.h"

class FirstDialog : public QDialog
{
	Q_OBJECT

public:
	FirstDialog(QWidget *parent = nullptr);
	~FirstDialog();
public slots:
	void onBtnClose();
signals:
	void sigStartApp();
private:
	Ui::FirstDialogClass ui;
};
