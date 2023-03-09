#pragma once

#include <QDialog>
#include <QLabel>
#include "ui_stopflydialog.h"

class StopFlyDialog : public QDialog
{
	Q_OBJECT

public:
	StopFlyDialog(QWidget *parent = nullptr);
	~StopFlyDialog();
private slots:
	void on_btnStopFly_clicked();
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
signals:
	void sigFlyControl();
private:
	Ui::StopFlyDialog ui;
	QLabel* m_pLabelBackground;
};
