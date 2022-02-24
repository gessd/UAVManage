#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_uavmanage.h"
#include <QDebug>
#include <QProcess>
#include <QWebSocket>
#include <QWebSocketServer>
//python宏定义与mavlink中宏定义有冲突，python引用必须放到mavlink引用之前
#include "threadpython.h"
#include "devicemanage.h"
#include "messagelistdialog.h"

class SoundGrade;
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
private slots:
    /**
    * @brief 新建项目
    */
    void onNewProject();
    /**
    * @brief 打开项目
    */
    void onOpenProject(QString qstrFile);
    //每一步都自动保存,无需手动保存,不自动保存会造成切换无人机后blockly新编辑内容丢失
    /**
    * @brief 项目另存为
    */
    void onSaveasProject();
    /**
     * @brief 清空WEB积木块
     */
    void onWebClear();
protected:
    virtual void showEvent(QShowEvent* event);
    virtual void closeEvent(QCloseEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
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
     * @brief 设备列表中设备新增或更新
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
    /**
    * @brief 设备起飞指令下发完成
    * @param 起飞|降落
    */
    void onDeviceTakeoffFinished(bool takeoff);
    /**
     * @brief 程序间通讯
     */
    void onAppMessage(const QString& message);
    /**
     * @brief 航点上传进度
     */
    void onWaypointProcess(QString name, unsigned int index, unsigned int count, int res, bool finish, QString text);
    /**
    * @brief 更新加载音乐文件
    */
    void onUpdateMusic(QString qstrFilePath);
    /**
    * @brief 当前音乐播放进度
    */
    void onCurrentMusicTime(int second);
    /**
    * @brief 当前音乐播放状态
    */
    void onCurrentPlayeState(qint8 state);
    /**
    * @brief 三维窗口连接状态
    */
    void on3DDialogStauts(bool connect);
private:
    /**
     * @brief 新建项目工程文件
     * @param [in] qstrFile 项目绝对路径名称
     * @param [in] X 场地大小
     * @param [in] Y 场地大小
     * @return 文件是否建立成功
     */
    bool newProjectFile(QString qstrFile, float X = 10.0, float Y = 10.0);
    /**
     * @brief 生成舞步
     * @param bUpload 是否上传舞步到设备
     */
    void deviceWaypoint(bool bUpload = false);
private:
    Ui::UAVManageClass ui;
    //设备列表
    DeviceManage* m_pDeviceManage;
    //websocket服务 积木块界面
    QWebSocketServer* m_pSocketServer;
    //web连接 积木块界面
    QWebSocket* m_pWebSocket;
    QString m_qstrLastWebMessage;
    //当前工程文件
    QString m_qstrCurrentProjectFile;
    ThreadPython ptyhon;
    SoundGrade* m_pSoundWidget;
    QProcess* m_p3DProcess;
};
