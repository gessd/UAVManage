#pragma once

#include <QDialog>
#include <QLabel>
#include "ui_registerdialog.h"

class RegisterDialog : public QDialog
{
	Q_OBJECT

public:
	RegisterDialog(QWidget *parent = nullptr);
	~RegisterDialog();
	bool isRegister();
protected:
	void showEvent(QShowEvent* event);
	void closeEvent(QCloseEvent* event);
	void keyPressEvent(QKeyEvent* event);
private:
	Ui::RegisterDialog ui;
	QLabel* m_pLabelBackground;
	bool m_bClose;
};
