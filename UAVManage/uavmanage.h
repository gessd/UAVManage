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
    void loadWeb();
    QString getCurrentBlocklyFile();
    QString getCurrentPythonFile();
public slots:
    void onNewProject();
    void onOpenProject(QString qstrFile);
    //每一步都自动保存,无需手动保存,不自动保存会造成切换无人机后blockly新编辑内容丢失
    void onSaveProject();
    void onSaveasProject();
    /**
     * @brief 情况WEB积木块
     */
    void onWebClear();
protected:
    virtual void showEvent(QShowEvent* event);
private slots:
    /**
     * @brief WEB页面加载进度，百分制
     */
    void onWebLoadProgress(int progress);
    /**
     * @brief WEB页面加载完成
     */
    void onWebLoadFinished(bool finished);
    /**
     * @brief WEB新建连接
     */
	void onSocketNewConnection();
    /**
     * @brief 收到WEB消息
     */
	void onSocketTextMessageReceived(QString message);
    /**
     * @brief WEB连接断开
     */
	void onSocketDisconnected();
    /**
     * @brief 选中设备改变
     * @param [in] currentName  新设备名称
     * @param [in] previousName 旧设备名称
     */
    void onCurrentDeviceNameChanged(QString currentName, QString previousName);
    /**
     * @brief 设备列表中设备新增
     */
    void onDeviceAdd(QString name, QString ip, float x, float y);
    /**
     * @brief 设备列表中设备移除
     */
    void onDeviceRemove(QString name);
    /**
     * @brief 设备列表中设备重命名
     */
    void onDeviceRename(QString newName, QString oldName);
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
    //当前工程文件
    QString m_qstrCurrentProjectFile;
};
