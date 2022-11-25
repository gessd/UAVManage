#include "aboutdialog.h"
#include <QGraphicsDropShadowEffect>
#include "definesetting.h"
#include "paramreadwrite.h"
#include <QDebug>

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool);
	this->setAttribute(Qt::WA_TranslucentBackground);
	QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
	shadow->setOffset(0, 0);
	shadow->setColor(QColor("#444444"));
	shadow->setBlurRadius(10);
	this->setGraphicsEffect(shadow);
	ui.labelVersion->setText("V " + AppVersion());
	ui.stackedWidget->setCurrentIndex(1);
	ui.radioButton->setChecked(ParamReadWrite::readParam(_Update_).toBool());
	connect(ui.btnClose, &QAbstractButton::clicked, [this]() {  accept(); });
	connect(ui.btnRetry, &QAbstractButton::clicked, [this]() {  onCheckNewVersion(); });
	connect(ui.btnCheckVersion, &QAbstractButton::clicked, [this]() {  onCheckNewVersion(); });
	connect(ui.btnUpdate, &QAbstractButton::clicked, [this]() {  onStartUpdate(); });
	connect(ui.radioButton, &QAbstractButton::clicked, [this]() {
		ParamReadWrite::writeParam(_Update_, ui.radioButton->isChecked());
		});
}

AboutDialog::~AboutDialog()
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
}

void AboutDialog::onCheckNewVersion()
{

}

void AboutDialog::onStartUpdate()
{

}

void AboutDialog::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->show();
	onCheckNewVersion();
}

void AboutDialog::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}
