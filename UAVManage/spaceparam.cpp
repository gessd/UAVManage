#include "spaceparam.h"
#include <QIntValidator>
#include <QGraphicsDropShadowEffect>

SpaceParam::SpaceParam(QWidget *parent)
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
	ui.lineEditX->setValidator(new QIntValidator(100, 10000, this));
	ui.lineEditY->setValidator(new QIntValidator(100, 10000, this));
	ui.lineEditX->setMaxLength(5);
	ui.lineEditY->setMaxLength(5);
	connect(ui.btnOK, &QAbstractButton::clicked, [this]() {accept();});
	connect(ui.btnCancel, &QAbstractButton::clicked, [this]() {  reject(); });
}

SpaceParam::~SpaceParam()
{}

unsigned int SpaceParam::getSpaceX()
{
	return ui.lineEditX->text().trimmed().toUInt();
}

unsigned int SpaceParam::getSpaceY()
{
	return ui.lineEditY->text().trimmed().toUInt();
}
