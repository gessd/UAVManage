#pragma once

#include <QWidget>
#include "ui_historymessage.h"
#include "messagelistdialog.h"

class HistoryMessage : public QWidget
{
	Q_OBJECT

public:
	HistoryMessage(QWidget *parent = nullptr);
	~HistoryMessage();
	void resetWidget();
private slots:
	void onMessageData(QString text, _Messagelevel level, bool clear);
private:
	Ui::HistoryMessage ui;
	bool m_bShowIng;
};
