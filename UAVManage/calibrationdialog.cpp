#include "calibrationdialog.h"

CalibrationDialog::CalibrationDialog(QMap<QString, DeviceControl*> map, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	QStringList keys = map.keys();
	foreach(QString name, keys) {
		QTreeWidgetItem* pItem = new QTreeWidgetItem;
		pItem->setCheckState(0, Qt::Unchecked);
		pItem->setText(0, name);
		ui.treeWidget->addTopLevelItem(pItem);
	}
}

CalibrationDialog::~CalibrationDialog()
{
}
