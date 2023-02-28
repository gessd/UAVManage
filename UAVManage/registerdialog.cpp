#include "registerdialog.h"
#include <QCloseEvent>
#include <QSettings>
#include <QMessageBox>

#define _KeyPath_ "HKEY_CLASSES_ROOT\\QZ\\UAV"
void getCpuInfo(unsigned int* CPUInfo, unsigned int InfoType)
{
#if defined(__GNUC__)// GCC  
	__cpuid(InfoType, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#elif defined(_MSC_VER)// MSVC  
#if _MSC_VER >= 1400 //VC2005才支持__cpuid  
	__cpuid((int*)(void*)CPUInfo, (int)(InfoType));
#else //其他使用getcpuidex  
	//getcpuidex(CPUInfo, InfoType, 0);
#endif  
#endif  
}

QString getCpuId()
{
	char pCpuId[32] = "";
	int dwBuf[4];
	getCpuInfo((unsigned int*)dwBuf, 1);
	sprintf(pCpuId, "%08X", dwBuf[3]);
	sprintf(pCpuId + 8, "%08X", dwBuf[0]);
	return QString(pCpuId);
}

RegisterDialog::RegisterDialog(QWidget *parent):QDialog(parent)
	//: QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint)
{
	ui.setupUi(this);
	m_bClose = false;
	m_pLabelBackground = nullptr;
	ui.lineEditID->setText(getCpuId());
	connect(ui.btnRegister, &QAbstractButton::clicked, [this]() {
		QSettings set(_KeyPath_, QSettings::NativeFormat);
		QString key = ui.lineEditKey->text().trimmed();
		set.setValue("key", key);
		if (key.isEmpty()) {
			QMessageBox::warning(this, tr("提示"), tr("授权码不能为空！"));
			return;
		}
		QMessageBox::information(this, tr("提示"), tr("软件授权成功"));
		close();
		});
}

RegisterDialog::~RegisterDialog()
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
}

bool RegisterDialog::isRegister()
{
	QSettings set(_KeyPath_, QSettings::NativeFormat);
	QString key = set.value("key").toString();
	return !key.isEmpty();
}

void RegisterDialog::showEvent(QShowEvent* event)
{
	QSettings set(_KeyPath_, QSettings::NativeFormat);
	QString key = set.value("key").toString();
	ui.lineEditKey->setText(key);
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 80%);font:60px;color:#FFFFFF;"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
	m_pLabelBackground->setText(tr("请授权注册后使用"));
	m_pLabelBackground->show();
}

void RegisterDialog::closeEvent(QCloseEvent* event)
{
	if (m_pLabelBackground)m_pLabelBackground->close();
	event->accept();
	if (false == isRegister())((QWidget*)parent())->close();
}

void RegisterDialog::keyPressEvent(QKeyEvent* event)
{
	if (!event) return;
	//屏蔽ESC关闭窗口
	if (Qt::Key_Escape == event->key()) return;
	QDialog::keyPressEvent(event);
}
