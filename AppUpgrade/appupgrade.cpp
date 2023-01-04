#include "appupgrade.h"
#include <QProcess>
#include <QFile>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QMovie>
#include <QMessageBox>
#include "../UAVManage/paramreadwrite.h"

AppUpgrade::AppUpgrade(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	connect(&m_process, &QProcess::readyReadStandardOutput, this, &AppUpgrade::onReadyReadStandardOutput);
	connect(&m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
	connect(&m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onErrorOccurred(QProcess::ProcessError)));
	ui.textBrowser->setVisible(false);
	m_pMovie = new QMovie(":/res/images/loading.gif");
	ui.labelGifLoading->setMovie(m_pMovie);
	m_pMovie->start();
}

AppUpgrade::~AppUpgrade()
{

}

int AppUpgrade::startUpdate()
{
    static bool bInit = false;
    if (bInit) return 0;
    bInit = true;
    QString qstrVersionFile = QApplication::applicationDirPath() + "/version.ini";
    if (false == QFile::exists(qstrVersionFile)) return -1;
	QString qstrNewFile = QApplication::applicationDirPath() + "/" + ParamReadWrite::readParam("file", "", "App", qstrVersionFile).toString();
    if (false == QFile::exists(qstrNewFile)) return -1;
    QString qstr7zFile = QApplication::applicationDirPath() + "/../7z/7z.exe";
    QString qstrParam = QString(" x -y %1 -o%2").arg(qstrNewFile).arg(QApplication::applicationDirPath()+"/../");
	qDebug() << "执行解压程序" << qstr7zFile + qstrParam;
	//m_process.setProgram("cmd");
	//QStringList argument;
	//argument << "/c" << qstr7zFile + qstrParam;
	//m_process.setArguments(argument);
	//m_process.start();
	//m_process.start("ping baidu.com -t");
	m_process.start(qstr7zFile + qstrParam);
	bool bStart = m_process.waitForStarted();
	qDebug() << "启动解压程序" << bStart;
	if (false == bStart) return -1;
    return 0;
}

void AppUpgrade::onReadyReadStandardOutput()
{
	QByteArray arrAll = m_process.readAllStandardOutput();
	qDebug() << "----" << QString::fromLocal8Bit(arrAll);
	ui.textBrowser->append(QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]:"));
	ui.textBrowser->append(QString::fromLocal8Bit(arrAll));
}

void AppUpgrade::onFinished(int code)
{
	ui.label->setText(tr("程序升级完成正在启动"));
	qDebug() << "程序升级完成";
	ui.textBrowser->append("程序升级完成");
	QProcess::startDetached(QApplication::applicationDirPath() + "/../UAVManage.exe");
	QTimer::singleShot(3000, this, SLOT(close()));
	//close();
}

void AppUpgrade::onErrorOccurred(QProcess::ProcessError error)
{
	qDebug() << "错误信息" << error;
}

void AppUpgrade::showEvent(QShowEvent* event)
{
    QTimer::singleShot(1000, [this]() {
		int nUpdate = startUpdate();
		if (nUpdate < 0) {
			qDebug() << "更新失败";
			m_pMovie->stop();
			ui.labelGifLoading->setVisible(false);
			ui.label->setText(tr("程序升级失败正在重启"));
			QMessageBox::warning(this, tr("提示"), tr("检查更新文件失败，请重试"));
			QProcess::startDetached(QApplication::applicationDirPath() + "/../UAVManage.exe");
			QTimer::singleShot(3000, this, SLOT(close()));
			return;
		}
        });
}
