#pragma once

#include <QDialog>
#include "ui_aboutdialog.h"

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	AboutDialog(QWidget *parent = nullptr);
	~AboutDialog();
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
private:
	Ui::AboutDialogClass ui;
	QLabel* m_pLabelBackground;
};
