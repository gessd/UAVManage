#include "aboutdialog.h"
#include <QGraphicsDropShadowEffect>
#include "definesetting.h"

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool);
	this->setAttribute(Qt::WA_TranslucentBackground);
	QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
	shadow->setOffset(0, 0);
	shadow->setColor(QColor("#444444"));
	shadow->setBlurRadius(10);
	this->setGraphicsEffect(shadow);
	ui.labelVersion->setText("V " + AppVersion());
	connect(ui.btnClose, &QAbstractButton::clicked, [this]() {  reject(); });
}

AboutDialog::~AboutDialog()
{}
