#include "uavmanage.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UAVManage w;
    w.show();
    return a.exec();
}
