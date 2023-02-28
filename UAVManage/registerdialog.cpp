#include "registerdialog.h"
#include <QCloseEvent>

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
	return !ui.lineEditKey->text().isEmpty();
}

void RegisterDialog::resizeEvent(QResizeEvent* event)
{
	if (m_pLabelBackground) {
		m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	}
}

void RegisterDialog::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 80%);font:60px;color:#FFFFFF;"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
	m_pLabelBackground->setText(tr("请注册授权后使用"));
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
