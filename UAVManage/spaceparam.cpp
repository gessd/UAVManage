#include "spaceparam.h"
#include <QIntValidator>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

SpaceParam::SpaceParam(bool create, QWidget *parent)
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
	if (create) {
		ui.dialogLabelTitle->setText(tr("新建项目"));
	}
	else {
		ui.dialogLabelTitle->setText(tr("项目属性"));
		ui.widgetProject->setEnabled(false);
		ui.frameName->setEnabled(false);
#ifndef _EditSpace_
		ui.frameSpace->setEnabled(false);
		ui.btnOK->setVisible(false);
#endif
	}
	connect(ui.btnOK, &QAbstractButton::clicked, [this]() {
		//检查输入值
		QString name = ui.lineEditName->text();
		if (name.isEmpty()) {
			QMessageBox::warning(this, tr("提示"), tr("未输入项目名称"));
			return;
		}
		QString path = ui.labelPath->text();
		if (path.isEmpty()) {
			QMessageBox::warning(this, tr("提示"), tr("未选择项目存放路径"));
			return;
		}
		QFileInfo fileInfo(path);
		if (false == fileInfo.isDir() || false == fileInfo.exists()) {
			QMessageBox::warning(this, tr("提示"), tr("选择路径不存在"));
			return;
		}
		QString dir = path + "/" + name;
		QFileInfo fileDir(dir);
		if (fileDir.exists()) {
			QMessageBox::warning(this, tr("提示"), tr("当前路径已存在相同名称文件或文件夹"));
			return;
		}
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
	connect(ui.btnPath, &QAbstractButton::clicked, [this]() {
		QString path = QFileDialog::getExistingDirectory(this, tr("选择目录"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QFileDialog::ShowDirsOnly);
		ui.labelPath->setText(path);
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

QString SpaceParam::getProjectPath()
{
	return ui.labelPath->text() + "/" + ui.lineEditName->text().trimmed();
}

void SpaceParam::setProjectPath(QString path)
{
	QFileInfo info(path);
	ui.lineEditName->setText(info.fileName());
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
