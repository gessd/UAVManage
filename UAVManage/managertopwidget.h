#pragma once

#include <QWidget>
#include "ui_managertopwidget.h"

class ManagerTopWidget : public QWidget
{
	Q_OBJECT

public:
	ManagerTopWidget(QWidget *parent = nullptr);
	~ManagerTopWidget();
	void addWidget(QWidget* pWidget);
	void removeWidget(QWidget* pWidget);
signals:
	void sigPrepareWidget();
private:
	Ui::ManagerTopWidget ui;
};
