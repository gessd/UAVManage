#include "registerdialog.h"
#include <QCloseEvent>
#include <QSettings>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include "sm4/sm4.h"

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

//秘钥字符串
//QZSYUAVMANAGEPRO
unsigned char key[16] = {
	0x51,0x5a,0x53,0x59,0x55,0x41,0x56,0x4d,
	0x41,0x4e,0x41,0x47,0x45,0x50,0x52,0x4f
};

RegisterDialog::RegisterDialog(QWidget *parent):QDialog(parent)
	//: QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint)
{
	ui.setupUi(this);
	m_bClose = false;
	m_pLabelBackground = nullptr;
	ui.lineEditID->setText(getCpuId());
	connect(ui.btnRegister, &QAbstractButton::clicked, [this]() {
		ui.labelError->clear();
		QString key = ui.lineEditKey->text().trimmed();
		if (key.isEmpty()) {
			QMessageBox::warning(this, tr("提示"), tr("授权码不能为空！"));
			return;
		}
		//QMessageBox::information(this, tr("提示"), tr("软件授权成功"));
		//close();
		QString id = ui.lineEditID->text().trimmed();
		QString error = DecipherKey(key, id);
		if (error.isEmpty()) {
			QSettings set(_KeyPath_, QSettings::NativeFormat);
			set.setValue("key", key);
			close();
			return;
		}
		qDebug() << error;
		ui.labelError->setText(error);
		QMessageBox::warning(this, tr("提示"), error);
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
	ui.labelError->clear();
	QSettings set(_KeyPath_, QSettings::NativeFormat);
	QString key = set.value("key").toString();
	QString id = ui.lineEditID->text().trimmed();
	QString error = DecipherKey(key, id);
	if (error.isEmpty())  return true;
	qDebug() << error;
	ui.labelError->setText(error);
	return false;
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

QString RegisterDialog::DecipherKey(QString text, QString& id)
{
	if (text.isEmpty()) {
		return tr("授权码为空无法解码");
	}
	QByteArray arrData = QByteArray::fromHex(text.toLatin1());
	if (arrData.length() != 16 * 2) {
		return tr("授权码长度错误");
	}
	qDebug() << "验证授权码" << text;
	SM4_KEY sm4_key;
	SM4_set_key(key, &sm4_key);
	uint8_t cbuf[16] = { 0 };
	uint8_t pbuf[16] = { 0 };
	QByteArray temp = arrData.left(16);
	memcpy(cbuf, temp, temp.length());
	SM4_decrypt(cbuf, pbuf, &sm4_key);
	QString tt = QByteArray((char*)pbuf, sizeof(pbuf));
	if (tt != id) {
		return tr("授权码与本机不匹配");
	}
	//qDebug() << pass;
	memset(cbuf, 0, sizeof(cbuf));
	memset(pbuf, 0, sizeof(pbuf));
	temp = arrData.right(16);
	memcpy(cbuf, temp, temp.length());
	SM4_decrypt(cbuf, pbuf, &sm4_key);
	QString date = QByteArray((char*)pbuf, sizeof(pbuf));
	QDateTime t = QDateTime::fromString(date, "yyyy-MM-dd"); 
	qDebug() << date << t;
	unsigned int current = QDateTime::currentDateTime().toTime_t();
	unsigned int v = t.toTime_t();
	qDebug() << "授权码到期时间" << v << current;
	int n = current - v;
	if (n > 0) {
		return tr("授权码已过期");
	}
	return "";
}

