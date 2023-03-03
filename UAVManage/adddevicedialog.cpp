#include "adddevicedialog.h"
#include "qlineedit.h"
#include <QRegExpValidator>

AddDeviceDialog::AddDeviceDialog(QString qstrName, unsigned int maxX, unsigned int maxY, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
	this->setAttribute(Qt::WA_TranslucentBackground);
	setName(qstrName);

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
		if (name.isEmpty()) return;
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
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()->parent()));
	//设置窗体的背景色,这里的百分比就是透明度
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent()->parent())->size());
	m_pLabelBackground->show();
}

void AddDeviceDialog::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}
