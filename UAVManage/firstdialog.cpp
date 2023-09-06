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
	QTimer::singleShot(1000, [this]() { emit sigStartApp(); });
	//自动关闭窗口，防止有错误提示时因窗口置顶造成遮挡
	QTimer::singleShot(3000, [this]() { close(); });
}

void FirstDialog::onBtnClose()
{
	close();
}
