#include "spaceparam.h"
#include <QIntValidator>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

SpaceParam::SpaceParam(bool init, QWidget *parent)
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
	ui.lineEditX->setValidator(new QIntValidator(5, 100, this));
	ui.lineEditY->setValidator(new QIntValidator(5, 100, this));
	ui.lineEditX->setMaxLength(2);
	ui.lineEditY->setMaxLength(2);
	if (init) {
		ui.dialogLabelTitle->setText(tr("新建项目"));
		ui.widgetProject->setVisible(false);
	}
	else {
		ui.dialogLabelTitle->setText(tr("项目属性"));
		ui.widgetProject->setVisible(true);
		ui.btnOK->setVisible(false);
	}
	connect(ui.btnOK, &QAbstractButton::clicked, [this]() {
		//检查输入值
		int x = ui.lineEditX->text().toInt();
		int y = ui.lineEditY->text().toInt();
		if (x < 5 || y < 5) {
			QMessageBox::warning(this, tr("提示"), tr("场地范围太小"));
			return;
		}
		if (x > 100 || y > 100) {
			QMessageBox::warning(this, tr("提示"), tr("场地范围太大"));
			return;
		}
		accept();
		});
	connect(ui.btnCancel, &QAbstractButton::clicked, [this]() {  reject(); });
}

SpaceParam::~SpaceParam()
{}

unsigned int SpaceParam::getSpaceX()
{
	return ui.lineEditX->text().trimmed().toUInt() * 100;
}

unsigned int SpaceParam::getSpaceY()
{
	return ui.lineEditY->text().trimmed().toUInt() * 100;
}

void SpaceParam::setProjectPath(QString path)
{
	ui.labelPath->setText(path);
}

void SpaceParam::setSpaceSize(unsigned int x, unsigned int y)
{
	ui.lineEditX->setText(QString::number(x / 100));
	ui.lineEditY->setText(QString::number(y / 100));
	ui.lineEditX->setEnabled(false);
	ui.lineEditY->setEnabled(false);
}
