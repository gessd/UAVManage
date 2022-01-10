#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_uavmanage.h"
#include <QDebug>
#include <QWebSocket>
#include <QWebSocketServer>
#include "devicemanage.h"

class UAVManage : public QMainWindow
{
    Q_OBJECT
   
public:
    UAVManage(QWidget *parent = Q_NULLPTR);
    ~UAVManage();
    void updateStyle();
    void updateWeb();
public slots:
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onSaveasProject();
protected:
    virtual void showEvent(QShowEvent* event);
private slots:
    //��ҳ���ؽ���
    void onWebLoadProgress(int progress);
    void onWebLoadFinished(bool finished);
	void onSocketNewConnection();
	void onSocketTextMessageReceived(const QString message);
	void onSocketDisconnected();
    void onCurrentDeviceNameChanged(QString currentName, QString previousName);
    void onBtnTestClicked();
private:
    Ui::UAVManageClass ui;
    //�豸�б�
    DeviceManage* m_pDeviceManage;
    //websocket����
    QWebSocketServer* m_pSocketServer;
    //web����
    QWebSocket* m_pWebSocket;
};
