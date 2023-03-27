#include "adddevicedialog.h"
#include "qlineedit.h"
#include <QRegExpValidator>
#include <QMessageBox>

AddDeviceDialog::AddDeviceDialog(QString qstrName, unsigned int maxX, unsigned int maxY, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
	this->setAttribute(Qt::WA_TranslucentBackground);
	setName(qstrName);
	m_maxX = maxX;
	m_maxY = maxY;
	ui.btnOK->setVisible(true);
	QRegExp regExp1("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
		"{3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	//限制输入
	ui.lineEditIP->setValidator(new QRegExpValidator(regExp1, this));
	ui.lineEditX->setValidator(new QIntValidator(100, maxX - 100, this));
	ui.lineEditY->setValidator(new QIntValidator(100, maxY - 100, this));
	ui.lineEditX->setMaxLength(5);
	ui.lineEditY->setMaxLength(5);
	connect(ui.btnOK, &QAbstractButton::clicked, [this]() { 
		QString name = ui.lineEditName->text().trimmed();
		if (name.isEmpty()) {
			QMessageBox::warning(this, tr("提示"), tr("请输入设备名称"));
			return;
		}
		int number = ui.lineEditX->text().toInt();
		if (number < 100 || number >(m_maxX - 100)) {
			QMessageBox::warning(this, tr("提示"), tr("X轴位置距离场地边界太近，最小100厘米"));
			return;
		}
		number = ui.lineEditY->text().toInt();
		if (number < 100 || number >(m_maxY - 100)) {
			QMessageBox::warning(this, tr("提示"), tr("Y轴位置距离场地边界太近，最小100厘米"));
			return;
		}
		accept();
		});
	connect(ui.btnCancel, &QAbstractButton::clicked, [this]() {  reject(); });
	connect(ui.lineEditName, &QLineEdit::textChanged, [this](QString text) {
		ui.btnOK->setVisible(true);
		if (text.isEmpty()) ui.btnOK->setVisible(false);
		});
	ui.lineEditX->setText(QString::number(100));
	ui.lineEditY->setText(QString::number(100));
}

AddDeviceDialog::~AddDeviceDialog()
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
}

void AddDeviceDialog::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	QWidget* pWidget = this;
	while (true) {
		QWidget* pTemp = dynamic_cast<QWidget*>(pWidget->parent());
		if(!pTemp) break;
		pWidget = pTemp;
	}
	m_pLabelBackground = new QLabel(pWidget);
	//设置窗体的背景色,这里的百分比就是透明度
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(pWidget->size());
	m_pLabelBackground->show();
}

void AddDeviceDialog::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}
