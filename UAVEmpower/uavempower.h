#pragma once

#include <QtWidgets/QWidget>
#include "ui_uavempower.h"

class UAVEmpower : public QWidget
{
    Q_OBJECT

public:
    UAVEmpower(QWidget *parent = nullptr);
    ~UAVEmpower();
private slots:
    void on_btnUpdate_clicked();
    void on_btnEncryption_clicked();
    void on_btnDeciphering_clicked();
private:
    Ui::UAVEmpowerClass ui;
};
