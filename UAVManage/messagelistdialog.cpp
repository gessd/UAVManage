#include "messagelistdialog.h"
#include <QTime>
#include <QDateTime>
#include <QColor>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QDebug>

//��Ϣ��ʼʱ��
#define _ItemTime_      Qt::UserRole+20
//��Ϣ��ʾͣ��ʱ��
#define _ItemWaitTime_  Qt::UserRole+21
//�б�ITEM�߶�
#define _ItemHeight_ 60
//�б���
#define _ItemSpacing 5

MessageListDialog* g_pMessage = NULL;
MessageListDialog* MessageListDialog::getInstance()
{
	if (g_pMessage) return g_pMessage;
	g_pMessage = new MessageListDialog;
	return g_pMessage;
}

void MessageListDialog::addMessageTest(QString text, _Messagelevel level)
{	
	qDebug() << text;
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
	pLabel->setWordWrap(true);
	pLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	unsigned int nMaxWidth = ui.listWidget->width() - 17;
	pLabel->setFixedSize(nMaxWidth, _ItemHeight_ - 2);
	switch (level)
	{
	case InfoMessage: 
		pItem->setData(_ItemWaitTime_, 3);
		pLabel->setObjectName("LabelInfoMessage");
		break;
	case WarningMessage:
		pItem->setData(_ItemWaitTime_, 6);
		pLabel->setObjectName("LabelWarningMessage");
		break;
	case ErrorMessage:
		pItem->setData(_ItemWaitTime_, 9);
		pLabel->setObjectName("LabelErrorMessage");
		break;
	default:
		break;
	}
	ui.listWidget->setItemWidget(pItem, pLabel);
	setFixedHeight(ui.listWidget->count() * (_ItemHeight_+ _ItemSpacing) + _ItemHeight_);

	//setWindowFlag(Qt::FramelessWindowHint, true);
	//setWindowFlag(Qt::Dialog, true);
	//setModal(true);

	show();
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
		setFixedHeight(ui.listWidget->count() * (_ItemHeight_ + _ItemSpacing) + _ItemHeight_);
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

