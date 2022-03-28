#include "uavmanage.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QMessageBox>
#include <QTranslator>
#include "qtsingleapplication.h"

void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
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
	}
	QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
	QString current_date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString message = QString("[%1]%2 %3").arg(current_date).arg(text).arg(msg);
	QString qstrFileName = QApplication::applicationDirPath()+"/Log/"+QDateTime::currentDateTime().toString("yyyyMMdd.log");
	QFile file(qstrFileName);
	if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
		QTextStream text_stream(&file);
		text_stream.setCodec("gbk");
		text_stream << message << "\r\n";
		file.flush();
		file.close();
	}
	mutex.unlock();
}
int main(int argc, char *argv[])
{
	QtSingleApplication a("myapp_id", argc, argv);
	if (a.isRunning())  //判断实例是否已经运行
	{
		qDebug() << "程序已运行";
		a.sendMessage("raise_window_noop", 1000);
		return EXIT_SUCCESS;
	}
	qDebug() << "程序启动";
	//注册MessageHandler
	//qInstallMessageHandler(outputMessage);
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
	//主程序
    UAVManage w;
    w.show();
	a.setActivationWindow(&w);
	QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(onAppMessage(const QString&)));
 	int n = a.exec();
	qDebug() << "程序退出";
    return n;
}
