#include "spaceparam.h"
#include <QIntValidator>
#include <QMessageBox>

SpaceParam::SpaceParam(bool init, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
	this->setAttribute(Qt::WA_TranslucentBackground);
	ui.lineEditX->setValidator(new QDoubleValidator(2.0, 100.0, 2, this));
	ui.lineEditY->setValidator(new QDoubleValidator(2.0, 100.0, 2, this));
	ui.lineEditX->setMaxLength(5);
	ui.lineEditY->setMaxLength(5);
	if (init) {
		ui.dialogLabelTitle->setText(tr("新建项目"));
		ui.widgetProject->setVisible(false);
	}
	else {
		ui.dialogLabelTitle->setText(tr("项目属性"));
		ui.widgetProject->setVisible(true);
		ui.btnOK->setVisible(false);
		ui.lineEditX->setEnabled(false);
		ui.lineEditY->setEnabled(false);
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
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
}

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
	ui.lineEditX->setText(QString::number(x / 100.0, 'f', 2));
	ui.lineEditY->setText(QString::number(y / 100.0, 'f', 2));
}

void SpaceParam::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->show();
}

void SpaceParam::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}
