#include "adddevicedialog.h"
#include "qlineedit.h"
#include <QRegExpValidator>

AddDeviceDialog::AddDeviceDialog(QString qstrName, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	setName(qstrName);

	ui.btnOK->setVisible(true);
	QRegExp regExp1("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
		"{3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	//ÏÞÖÆÊäÈë
	ui.lineEditIP->setValidator(new QRegExpValidator(regExp1, this));
	ui.lineEditX->setValidator(new QDoubleValidator(0, 500, 2, this));
	ui.lineEditY->setValidator(new QDoubleValidator(0, 500, 2, this));
	ui.lineEditX->setMaxLength(6);
	ui.lineEditY->setMaxLength(6);
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
