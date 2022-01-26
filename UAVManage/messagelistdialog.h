#pragma once

#include <QDialog>
#include "ui_messagelistdialog.h"
#include <QTimer>

enum _Messagelevel
{
	InfoMessage,
	WarningMessage,
	ErrorMessage
};
#define _ShowInfoMessage(M) MessageListDialog::getInstance()->addMessageTest(M)
#define _ShowWarningMessage(M) MessageListDialog::getInstance()->addMessageTest(M,WarningMessage)
#define _ShowErrorMessage(M) MessageListDialog::getInstance()->addMessageTest(M, ErrorMessage)

class MessageListDialog : public QDialog
{
	Q_OBJECT
public:
	static MessageListDialog* getInstance();
	void addMessageTest(QString text, _Messagelevel level= InfoMessage);
	void exitDialog();
private:
	MessageListDialog(QWidget *parent = Q_NULLPTR);
	~MessageListDialog();
private:
	Ui::MessageListDialog ui;
	QTimer m_timer;
};
