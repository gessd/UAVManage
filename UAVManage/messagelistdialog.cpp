#include "messagelistdialog.h"
#include <QTime>
#include <QDateTime>
#include <QColor>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QDebug>

//消息初始时间
#define _ItemTime_      Qt::UserRole+20
//消息显示停留时间
#define _ItemWaitTime_  Qt::UserRole+21
//列表ITEM高度
#define _ItemHeight_ 60
//列表间隔
#define _ItemSpacing 5
//界面顶部固定高度
#define _TitleHeight 60

MessageListDialog* g_pMessage = NULL;
MessageListDialog* MessageListDialog::getInstance()
{
	if (g_pMessage) return g_pMessage;
	g_pMessage = new MessageListDialog;
	return g_pMessage;
}

void MessageListDialog::addMessageTest(QString text, _Messagelevel level)
{
	show();
	if (parent()) {
		QWidget* pWidget = dynamic_cast<QWidget*>(parent());
		if (pWidget) {
			move((pWidget->width() - width()) / 2, 0);
		}
	}
	
	QListWidgetItem* pItem = new QListWidgetItem();
	ui.listWidget->addItem(pItem);
	pItem->setData(_ItemTime_, QDateTime::currentDateTime().toTime_t());
	QString qstrTextStyle = "color:rgb(250,250,250);background-color:rgb(100,100,100);border-radius:5px;";
	switch (level)
	{
	case InfoMessage: 
		pItem->setData(_ItemWaitTime_, 10);
		break;
	case WarningMessage:
		pItem->setData(_ItemWaitTime_, 20);
		pItem->setTextColor(QColor(Qt::yellow));
		qstrTextStyle = "color:rgb(250,250,50);background-color:rgb(100,100,100);border-radius:5px;";
		break;
	case ErrorMessage:
		pItem->setData(_ItemWaitTime_, 30);
		pItem->setTextColor(QColor(Qt::red));
		qstrTextStyle = "color:rgb(250,100,100);background-color:rgb(100,100,100);border-radius:5px;";
		break;
	default:
		break;
	}
	pItem->setSizeHint(QSize(0, _ItemHeight_));

	QLabel* pLabel = new QLabel(text);
	pLabel->setWordWrap(true);
	pLabel->setStyleSheet(qstrTextStyle);
	unsigned int nMaxWidth = ui.listWidget->width() - 17;
	pLabel->setFixedSize(nMaxWidth, _ItemHeight_ - 2);
	//QWidget* pWidget = new QWidget;
	//QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
	//pWidget->setLayout(pLayout);
	//pLayout->addWidget(pLabel);
	ui.listWidget->setItemWidget(pItem, pLabel);

	setFixedHeight(ui.listWidget->count() * (_ItemHeight_+ _ItemSpacing) + _TitleHeight);
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
		setFixedHeight(ui.listWidget->count() * (_ItemHeight_ + _ItemSpacing) + _TitleHeight);
		if (0 == ui.listWidget->count()) close();
		});
	connect(ui.listWidget, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
		ui.listWidget->takeItem(ui.listWidget->currentRow());
		});
	connect(ui.btnClose, &QAbstractButton::clicked, [this]() {close();});
}

MessageListDialog::~MessageListDialog()
{
}

