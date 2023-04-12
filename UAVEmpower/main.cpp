#include "uavempower.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UAVEmpower w;
    w.show();
    return a.exec();
}
