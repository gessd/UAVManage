#include "uavmanage.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include <QFile>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	QTextCodec* codec = QTextCodec::codecForName("utf-8");
	QTextCodec::setCodecForLocale(codec);

    QFile file(":/res/qss/style.qss");
    if (file.open(QIODevice::ReadOnly)) {
        //qApp->setStyleSheet(file.readAll());
        file.close();
    }

    UAVManage w;
    w.show();
    return a.exec();
}
