#include "firstdialog.h"
#include <QDebug>

FirstDialog::FirstDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
	connect(ui.btnClose, &QAbstractButton::clicked, this, &FirstDialog::onBtnClose);
	connect(ui.btnStart, &QAbstractButton::clicked, [this]() { 
		emit sigStartApp(); 
		});
}

FirstDialog::~FirstDialog()
{
}

void FirstDialog::onBtnClose()
{
	close();
}
