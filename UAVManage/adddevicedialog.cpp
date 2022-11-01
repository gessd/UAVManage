#include "adddevicedialog.h"
#include "qlineedit.h"
#include <QRegExpValidator>
#include <QGraphicsDropShadowEffect>

AddDeviceDialog::AddDeviceDialog(QString qstrName, QWidget *parent)
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
	setName(qstrName);

	ui.btnOK->setVisible(true);
	QRegExp regExp1("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
		"{3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	//ÏÞÖÆÊäÈë
	ui.lineEditIP->setValidator(new QRegExpValidator(regExp1, this));
	ui.lineEditX->setValidator(new QIntValidator(10, 10000, this));
	ui.lineEditY->setValidator(new QIntValidator(10, 10000, this));
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
}

AddDeviceDialog::~AddDeviceDialog()
{
}
