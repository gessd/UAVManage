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
	void on_btnFlyLandCont_clicked();
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
signals:
	void sigFlyControl(bool stop);
private:
	Ui::StopFlyDialog ui;
	QLabel* m_pLabelBackground;
};
