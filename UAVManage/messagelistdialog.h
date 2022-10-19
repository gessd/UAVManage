#pragma once

#include <QDialog>
#include "ui_messagelistdialog.h"
#include <QTimer>

enum _Messagelevel
{
	InfoMessage = 1,
	WarningMessage = 2,
	ErrorMessage =3
};
#define _MessageListClear MessageListDialog::getInstance()->clearMessageList();
#define _ShowInfoMessage(M) MessageListDialog::getInstance()->addMessageTest(M)
#define _ShowWarningMessage(M) MessageListDialog::getInstance()->addMessageTest(M,WarningMessage)
#define _ShowErrorMessage(M) MessageListDialog::getInstance()->addMessageTest(M, ErrorMessage)

class MessageListDialog : public QDialog
{
	Q_OBJECT
public:
	static MessageListDialog* getInstance();
	void addMessageTest(QString text, _Messagelevel level= InfoMessage);
	void clearMessageList();
	void exitDialog();
private:
	MessageListDialog(QWidget *parent = Q_NULLPTR);
	~MessageListDialog();
private slots:
	void onShowMessageText(QString text, _Messagelevel level, bool clear);
signals:
	void sigMessage(QString text, _Messagelevel level, bool clear);
private:
	Ui::MessageListDialog ui;
	QTimer m_timer;
};
