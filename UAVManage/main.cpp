#include "uavmanage.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include <QFile>
#include <QTextCodec>

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
		text_stream << message << "\r\n";
		file.flush();
		file.close();
	}
	mutex.unlock();
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	//×¢²áMessageHandler
	//qInstallMessageHandler(outputMessage);

	QTextCodec* codec = QTextCodec::codecForName("utf-8");
	QTextCodec::setCodecForLocale(codec);

    QFile file(":/res/qss/style.qss");
    if (file.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }

    UAVManage w;
    w.show();
    return a.exec();
}
