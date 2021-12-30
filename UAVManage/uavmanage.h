#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_uavmanage.h"
#include <QDebug>
#include "devicemanage.h"

class UAVManage : public QMainWindow
{
    Q_OBJECT
   
public:
    UAVManage(QWidget *parent = Q_NULLPTR);

private slots:
    void onBtnTestClicked();
private:
    Ui::UAVManageClass ui;
    DeviceManage* m_pDeviceManage;
};
