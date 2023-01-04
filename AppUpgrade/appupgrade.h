#pragma once

#include <QtWidgets/QMainWindow>
#include <QProcess>
#include <QMovie>
#include "ui_appupgrade.h"

class AppUpgrade : public QMainWindow
{
    Q_OBJECT

public:
    AppUpgrade(QWidget *parent = nullptr);
    ~AppUpgrade();
    int startUpdate();
private slots:
    void onReadyReadStandardOutput();
    void onFinished(int code);
    void onErrorOccurred(QProcess::ProcessError error);
protected:
	void showEvent(QShowEvent* event);
private:
    Ui::AppUpgradeClass ui;
    QProcess m_process;
    QMovie* m_pMovie;
};
