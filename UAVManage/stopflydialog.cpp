#include "stopflydialog.h"
#include <QDebug>

StopFlyDialog::StopFlyDialog(QWidget *parent) : QDialog(parent)
	//: QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	//setAutoFillBackground(true);
	//QPalette pal = palette();
	//pal.setColor(QPalette::Background, Qt::transparent);
	//setPalette(pal);
	//setWindowOpacity(1);
	//setAttribute(Qt::WA_TranslucentBackground, true);
}

StopFlyDialog::~StopFlyDialog()
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
}

void StopFlyDialog::on_btnStopFly_clicked()
{
	emit sigFlyControl(true);
}

void StopFlyDialog::on_btnFlyLandCont_clicked()
{
	emit sigFlyControl(false);
}

void StopFlyDialog::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 80%);"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->show();
}

void StopFlyDialog::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}
