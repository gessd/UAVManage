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
	QtSingleApplication a("UAVManage", argc, argv);
#ifdef Q_OS_WIN
	//抛出异常
	::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ExceptionCrashHandler);
#endif
	//注册MessageHandler
	qInstallMessageHandler(outputMessage);
	//判断启动参数
	QStringList listParam;
	for (int i = 0; i < argc; i++) {
		listParam.append(argv[i]);
	}
	qInfo() << listParam;
	if (2 != listParam.count()) {
		qWarning() << "程序无启动参数";
		return -1;
	}
	if (QString("p210") != listParam.at(1)) {
		qWarning() << "程序启动参数错误";
		return -1;
	}

	//判断实例是否已经运行
	if (a.isRunning()) {
		QTime time;
		time.start();
		while (time.elapsed() < 1000) {
			//如果程序正在运行则等1秒，防止切换模式时会操作程序未完全退出造成无法启动
			QApplication::processEvents();
		}
		if (a.isRunning()) {
			qInfo() << "程序已运行";
			a.sendMessage("raise_window_noop", 1000);
			return EXIT_SUCCESS;
		}
	}

#ifdef _UseUWBData_
	qInfo() << "使用UWB基站模式";
	ParamReadWrite::writeParam("UWB", true);
#else
	//默认启动为WIFI模式，所以先读取配置，如果上一次用的是UWB模式则启动UWB程序
	bool bUseUwb = ParamReadWrite::readParam("UWB", false).toBool();
	if (bUseUwb) {
		QProcess* process = new QProcess;
		process->start("UAVManage-UWB.exe");
		return 0;
	}
	qInfo() << "使用WIFI网络模式";
	ParamReadWrite::writeParam("UWB", false);
#endif

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
