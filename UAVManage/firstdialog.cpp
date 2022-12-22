#include "firstdialog.h"
#include <QDebug>
#include <QTimer>

FirstDialog::FirstDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
	ui.widgetTitle->setVisible(false);
	connect(ui.btnClose, &QAbstractButton::clicked, this, &FirstDialog::onBtnClose);
	connect(ui.btnStart, &QAbstractButton::clicked, [this]() { emit sigStartApp(); });
}

FirstDialog::~FirstDialog()
{
}

void FirstDialog::showEvent(QShowEvent* event)
{
	QTimer::singleShot(500, [this]() { emit sigStartApp(); });
}

void FirstDialog::onBtnClose()
{
	close();
}
