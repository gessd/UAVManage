#include "uavmanage.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile file(":/res/qss/style.qss");
    if (file.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }

    UAVManage w;
    w.show();
    return a.exec();
}
