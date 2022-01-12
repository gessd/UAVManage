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
    void onOpenProject(QString qstrFile);
    //每一步都自动保存,无需手动保存,不自动保存会造成切换无人机后blockly新编辑内容丢失
    void onSaveProject();
    void onSaveasProject();
protected:
    virtual void showEvent(QShowEvent* event);
private slots:
    //网页加载进度
    void onWebLoadProgress(int progress);
    void onWebLoadFinished(bool finished);
	void onSocketNewConnection();
	void onSocketTextMessageReceived(const QString message);
	void onSocketDisconnected();
    void onCurrentDeviceNameChanged(QString currentName, QString previousName);
    void onBtnTestClicked();
private:
    /**
     * @brief 新建项目工程文件
     * @param [in] qstrFile 项目绝对路径名称
     * @param [in] X 场地大小
     * @param [in] Y 场地大小
     * @return 文件是否建立成功
     */
    bool newProjectFile(QString qstrFile, float X = 10.0, float Y = 10.0);
private:
    Ui::UAVManageClass ui;
    //设备列表
    DeviceManage* m_pDeviceManage;
    //websocket服务
    QWebSocketServer* m_pSocketServer;
    //web连接
    QWebSocket* m_pWebSocket;
};
