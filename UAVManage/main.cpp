#include "uavmanage.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QMessageBox>
#include <QTranslator>
#include "qtsingleapplication.h"
#include "paramreadwrite.h"
#include "firstdialog.h"

void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	static QString qstrLast = "";
	if (msg == qstrLast) return;
	static QMutex mutex;
	mutex.lock();
	QString text;
	switch (type)
	{
	case QtDebugMsg:
		text = QString("Debug:");
		break;
	case QtWarningMsg:
		text = QString("Warning:");
		break;
	case QtCriticalMsg:
		text = QString("Critical:");
		break;
	case QtFatalMsg:
		text = QString("Fatal:");
		break;
	case QtInfoMsg:
		text = QString("Info:");
		break;
	}
	QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
	QString current_date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString message = QString("[%1]%2 %3").arg(current_date).arg(text).arg(msg);
	QString qstrFileName = QApplication::applicationDirPath()+"/Log/"+QDateTime::currentDateTime().toString("yyyyMMdd.log");
	QFile file(qstrFileName);
	if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
		QTextStream text_stream(&file);
		text_stream.setCodec("utf-8");
		text_stream << message << "\r\n";
		file.flush();
		file.close();
		qstrLast = msg;
	}
	mutex.unlock();
}

bool checkVersion() 
{
	QString config = QString("%1%2/%3").arg(QApplication::applicationDirPath()).arg(_NewVersionPath_).arg(_VersionFile_);
	QString qstrNewVersionNumber = ParamReadWrite::readParam("version", AppVersion(), _Root_, config).toString();
	qInfo() << "记录的版本号" << qstrNewVersionNumber;
	QStringList list = qstrNewVersionNumber.split(".");
	if (list.count() != 3) {
		return false;
	}
	bool bUpdate = false;
	if (list.at(0).toUInt() > _MajorNumber_) {
		bUpdate = true;
	}
	else if (list.at(0).toUInt() >= _MajorNumber_ && list.at(1).toUInt() > _MinorNumber_) {
		bUpdate = true;
	}
	else if (list.at(0).toUInt() >= _MajorNumber_ && list.at(1).toUInt() >= _MinorNumber_ && list.at(2).toUInt() > _BuildNumber_) {
		bUpdate = true;
	}
	else {
		return false;
	}
	if (bUpdate) {
		QString qstrFileName = ParamReadWrite::readParam("file", AppVersion(), _Root_, config).toString();
		QString qstrFilePath = QApplication::applicationDirPath() + _NewVersionPath_ + "/" + qstrFileName;
		if (false == QFile::exists(qstrFilePath)) return false;
		QProcess process;
		qInfo() << "启动新版本安装程序" << qstrNewVersionNumber << qstrFilePath;
		process.startDetached(qstrFilePath);
		process.waitForStarted(1000);
		return true;
	}
	return false;
}

#ifdef Q_OS_WIN
#include <imagehlp.h>
#pragma comment(lib, "DbgHelp.lib")
LONG ExceptionCrashHandler(EXCEPTION_POINTERS* pException)
{
	// 创建Dump文件
	HANDLE hDumpFile = CreateFileW(L"app.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// Dump信息
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pException;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = TRUE;
	// 写入Dump文件内容
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
	CloseHandle(hDumpFile);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char *argv[])
{
	QtSingleApplication a("myapp_id", argc, argv);
#ifdef Q_OS_WIN
	//抛出异常
	::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ExceptionCrashHandler);
#endif
	//注册MessageHandler
	qInstallMessageHandler(outputMessage);
	if (a.isRunning())  //判断实例是否已经运行
	{
		qInfo() << "程序已运行";
		a.sendMessage("raise_window_noop", 1000);
		return EXIT_SUCCESS;
	}
	qInfo() << "--------------------程序启动--------------------" << AppVersion() << __DATE__ << __TIME__;
	//添加翻译文件，用于界面控件中的英文翻译
	QTranslator translator;
	if (translator.load(":/res/translations/qt_zh_CN.qm")) {
		a.installTranslator(&translator);
	}
	//添加样式文件
    QFile file(":/res/qss/style.qss");
    if (file.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }
	//if (checkVersion()) {
	//	qInfo() << "有新版本程序文件，关闭程序";
	//	return 0;
	//}
	//deleteDir(QApplication::applicationDirPath()+_NewVersionPath_);
	//引导界面
	FirstDialog first;
	a.setActivationWindow(&first);
	first.show();
	UAVManage w;
	QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(onAppMessage(const QString&)));
	QObject::connect(&w, &UAVManage::sigWindowFinished, [&]() { a.setActivationWindow(&w); first.close(); });
	QObject::connect(&first, &FirstDialog::sigStartApp, [&]() { 
		//提示框Tip消息使用
		a.installEventFilter(&w);
		w.show(); });
	int n = a.exec();
	qInfo() << "====================程序退出====================" << n;
    return n;
}
