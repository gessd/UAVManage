#pragma once

#include <QDialog>
#include "ui_aboutdialog.h"

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	AboutDialog(QWidget *parent = nullptr);
	~AboutDialog();
public slots:
	void onCheckNewVersion();
	void onStartUpdate();
	void backgroundShow();
	void onRestartApp();
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
private:
	Ui::AboutDialogClass ui;
	QLabel* m_pLabelBackground;
	QString m_qstrNewVersionName;
	bool m_bShowing;
	bool m_bAutoUpdate;
};
