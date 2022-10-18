#include "spaceparam.h"
#include <QIntValidator>

SpaceParam::SpaceParam(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	ui.lineEditX->setValidator(new QIntValidator(0, 10000, this));
	ui.lineEditY->setValidator(new QIntValidator(0, 10000, this));
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
