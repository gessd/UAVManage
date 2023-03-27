#include "historymessage.h"
#include <QDebug>
#include <QDateTime>
#include <QGraphicsDropShadowEffect>

HistoryMessage::HistoryMessage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_bShowIng = false;
	QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(ui.textBrowser);
	shadow->setOffset(0, 0);
	shadow->setColor(QColor("#444444"));
	shadow->setBlurRadius(10);
	ui.textBrowser->setGraphicsEffect(shadow);
	ui.btnHide->setVisible(m_bShowIng);
	ui.btnShow->setVisible(!m_bShowIng);
	connect(ui.btnShow, &QAbstractButton::clicked, [this]() { m_bShowIng = true; resetWidget(); });
	connect(ui.btnHide, &QAbstractButton::clicked, [this]() { m_bShowIng = false; resetWidget(); });
	resetWidget();
}

HistoryMessage::~HistoryMessage()
{
}

void HistoryMessage::resetWidget()
{
	QWidget* pWidget = dynamic_cast<QWidget*>(parent());
	if (!pWidget) return;
	if (pWidget->height() <= 0 && pWidget->width() <= 0) return;
	ui.btnHide->setVisible(m_bShowIng);
	ui.btnShow->setVisible(!m_bShowIng);
	this->setFixedWidth(pWidget->width() / 2);
	if (m_bShowIng) {
		this->setGeometry(pWidget->width() / 2 - width() / 2, 0, width(), pWidget->height() / 2);
	}
	else {
		this->setGeometry(pWidget->width() / 2 - width() / 2, ui.widgetButton->height() - height(), width(), height());
	}
}

void HistoryMessage::onMessageData(QString text, _Messagelevel level, bool clear)
{
	QString qstrTime = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]:");
	QString qstrColor = "#000000";
	switch (level)
	{
	case InfoMessage:
		break;
	case WarningMessage:
		break;
	case ErrorMessage:
		qstrColor = "#FF0000";
		break;
	default:
		break;
	}
	QString qstrCss = QString(qstrTime+"<font color=%1>%2</font>").arg(qstrColor).arg(text);
	ui.textBrowser->append(qstrCss);
	if (clear) {
		ui.textBrowser->clear();
	}
}