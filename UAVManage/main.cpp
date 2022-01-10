#include "uavmanage.h"
#include <QtWidgets/QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UAVManage w;
    w.show();
    return a.exec();
}
