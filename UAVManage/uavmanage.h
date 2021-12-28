#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_uavmanage.h"

class UAVManage : public QMainWindow
{
    Q_OBJECT

public:
    UAVManage(QWidget *parent = Q_NULLPTR);

private:
    Ui::UAVManageClass ui;
};
