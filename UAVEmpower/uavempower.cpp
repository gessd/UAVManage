#include "uavempower.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <openssl/e_os2.h>
#include "sm4.h"

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

UAVEmpower::UAVEmpower(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
	setStyleSheet("font:14px;");
	on_btnUpdate_clicked();
}

UAVEmpower::~UAVEmpower()
{}

void UAVEmpower::on_btnUpdate_clicked()
{
	QString id = getCpuId();
	ui.lineEditID->setText(id);
	ui.lineEditKey->clear();
	ui.lineEditText->clear();
	ui.dateEdit->setDate(QDateTime::currentDateTime().date());
}

void UAVEmpower::on_btnEncryption_clicked()
{
	QString id = ui.lineEditID->text().trimmed();
	QString date = ui.dateEdit->text();
	QString text = QString("%1|%2").arg(id).arg(date);
	//qDebug() << id << date << text;
	if (id.length() != 16 ) {
		QMessageBox::warning(this, tr("错误"), tr("设备码长度错误"));
		return;
	}
	//设置秘钥
	SM4_KEY sm4_key;
	SM4_set_key(key, &sm4_key);
	uint8_t cbuf[16] = { 0 };
	uint8_t pbuf[16] = { 0 };
	uint8_t mbuf[16] = { 0 };
	strcpy((char*)mbuf, id.toStdString().c_str());
	//加密
	SM4_encrypt(mbuf, cbuf, &sm4_key);
	QByteArray pass((char*)cbuf, sizeof(cbuf));
	pass = pass.toHex().toUpper();
	//qDebug()  << pass;

	////解密
	//SM4_decrypt(cbuf, pbuf, &sm4_key);
	//QByteArray tt((char*)pbuf, sizeof(pbuf));
	//qDebug() << tt;

	memset(cbuf, 0, sizeof(cbuf));
	memset(pbuf, 0, sizeof(pbuf));
	memset(mbuf, 0, sizeof(mbuf));
	strcpy((char*)mbuf, date.toStdString().c_str());
	SM4_encrypt(mbuf, cbuf, &sm4_key);
	QByteArray validity((char*)cbuf, sizeof(cbuf));
	validity = validity.toHex().toUpper();
	//qDebug() << validity;
	ui.lineEditKey->setText(pass + validity);

	//SM4_decrypt(cbuf, pbuf, &sm4_key);
	//QByteArray aa((char*)pbuf, sizeof(pbuf));
	//qDebug() << aa;
}

void UAVEmpower::on_btnDeciphering_clicked()
{
	QString text = ui.lineEditKey->text().trimmed();
	if (text.isEmpty()) return;
	//qDebug() << text;
	QByteArray arrData = QByteArray::fromHex(text.toLatin1());
	if (arrData.length() != 16 * 2) {
		QMessageBox::warning(this, tr("错误"), tr("授权码长度错误"));
		return;
	}
	SM4_KEY sm4_key;
	SM4_set_key(key, &sm4_key);
	uint8_t cbuf[16] = { 0 };
	uint8_t pbuf[16] = { 0 };
	QByteArray temp = arrData.left(16);
	strcpy((char*)cbuf, temp.data());
	SM4_decrypt(cbuf, pbuf, &sm4_key);
	QString pass = QByteArray((char*)pbuf, sizeof(pbuf)); ;
	//qDebug() << pass;
	memset(cbuf, 0, sizeof(cbuf));
	memset(pbuf, 0, sizeof(pbuf));
	temp = arrData.right(16);
	strcpy((char*)cbuf, temp.data());
	SM4_decrypt(cbuf, pbuf, &sm4_key);
	QString validity = QByteArray((char*)pbuf, sizeof(pbuf));
	//qDebug() << validity;
	QString t = pass + " " + validity;
	ui.lineEditText->setText(t);
	QDateTime d = QDateTime::fromString(validity, "yyyy-MM-dd");
	//qDebug() << d;
}

