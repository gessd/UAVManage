#include "messagelistdialog.h"
#include <QTime>
#include <QDateTime>
#include <QColor>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

//消息初始时间
#define _ItemTime_      Qt::UserRole+20
//消息显示停留时间
#define _ItemWaitTime_  Qt::UserRole+21
//列表ITEM高度
#define _ItemHeight_ 60
//列表间隔
#define _ItemSpacing 5

MessageListDialog* g_pMessage = NULL;
MessageListDialog* MessageListDialog::getInstance()
{
	if (g_pMessage) return g_pMessage;
	g_pMessage = new MessageListDialog;
	g_pMessage->close();
	return g_pMessage;
}

void MessageListDialog::addMessageTest(QString text, _Messagelevel level)
{	
	emit sigMessage(text, level, false);
}

void MessageListDialog::clearMessageList()
{
	emit sigMessage("", InfoMessage, true);
}

void MessageListDialog::exitDialog()
{
	if (g_pMessage) {
		g_pMessage->close();
		delete g_pMessage;
		g_pMessage = nullptr;
	}
}

MessageListDialog::MessageListDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_timer.start(1000);
	ui.listWidget->setSpacing(_ItemSpacing);
	qRegisterMetaType<_Messagelevel>("_Messagelevel");
	//通过消息发送出去再显示是为了防止在子线程中显示错误信息操作崩溃
	connect(this, &MessageListDialog::sigMessage, this, &MessageListDialog::onShowMessageText);
	//定时删掉历史提示
	connect(&m_timer, &QTimer::timeout, [this]() {
		int count = ui.listWidget->count();
		for (int i = 0; i < count; i++) {
			QListWidgetItem* pItem = ui.listWidget->item(i);
			if(!pItem) continue;
			unsigned int nTime = pItem->data(_ItemTime_).toUInt();
			unsigned int nCurrent = QDateTime::currentDateTime().toTime_t();
			unsigned int nWait = pItem->data(_ItemWaitTime_).toUInt();
			if(nCurrent - nTime < nWait) continue;
			ui.listWidget->takeItem(i);
		}
		setFixedHeight(ui.listWidget->count() * (_ItemHeight_ + _ItemSpacing) + _ItemHeight_);
		if (0 == ui.listWidget->count()) close();
		});
	connect(ui.listWidget, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
		ui.listWidget->takeItem(ui.listWidget->currentRow());
		});
	connect(ui.btnClose, &QAbstractButton::clicked, [this]() {close();});
	ui.widgetMessageTitle->setVisible(false);
}

MessageListDialog::~MessageListDialog()
{
}

void MessageListDialog::onShowMessageText(QString text, _Messagelevel level, bool clear)
{
	//提升窗口置顶显示
	raise();
	if (true == clear) {
		int count = ui.listWidget->count();
		qDebug() << "清空提示消息" << count;
		ui.listWidget->clear();
		setFixedHeight(_ItemHeight_);
		close();
		return;
	}

	if (parent()) {
		QWidget* pWidget = dynamic_cast<QWidget*>(parent());
		if (pWidget) {
			move((pWidget->width() - width()) / 2, 0);
		}
	}

	QListWidgetItem* pItem = new QListWidgetItem();
	ui.listWidget->addItem(pItem);
	pItem->setSizeHint(QSize(0, _ItemHeight_));
	pItem->setData(_ItemTime_, QDateTime::currentDateTime().toTime_t());
	QLabel* pLabel = new QLabel(text);
	QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
	shadow->setOffset(0, 0);
	shadow->setColor(QColor("#444444"));
	shadow->setBlurRadius(10);
	pLabel->setGraphicsEffect(shadow);
	pLabel->setWordWrap(true);
	pLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	unsigned int nMaxWidth = ui.listWidget->width() - 12;
	pLabel->setFixedSize(nMaxWidth, _ItemHeight_ - 2);
	switch (level)
	{
	case InfoMessage:
		pItem->setData(_ItemWaitTime_, 3);
		pLabel->setObjectName("LabelInfoMessage");
		qInfo() << "普通消息提示" << text;
		break;
	case WarningMessage:
		pItem->setData(_ItemWaitTime_, 6);
		pLabel->setObjectName("LabelWarningMessage");
		qWarning() << "警告消息提示" << text;
		break;
	case ErrorMessage:
		pItem->setData(_ItemWaitTime_, 9);
		pLabel->setObjectName("LabelErrorMessage");
		qWarning() << "错误消息提示" << text;
		break;
	default:
		break;
	}
	ui.listWidget->setItemWidget(pItem, pLabel);
	setFixedHeight(ui.listWidget->count() * (_ItemHeight_ + _ItemSpacing) + _ItemHeight_);

	//setWindowFlag(Qt::FramelessWindowHint, true);
	//setWindowFlag(Qt::Dialog, true);
	//setModal(true);

	show();
}

