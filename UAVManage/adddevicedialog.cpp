#include "adddevicedialog.h"
#include "qlineedit.h"

AddDeviceDialog::AddDeviceDialog(QString qstrName, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	setName(qstrName);

	ui.btnOK->setVisible(true);
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
