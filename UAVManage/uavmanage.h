#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_uavmanage.h"
#include <QDebug>
#include <QWebSocket>
#include <QWebSocketServer>
//python�궨����mavlink�к궨���г�ͻ��python���ñ���ŵ�mavlink����֮ǰ
#include "threadpython.h"
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
    //ÿһ�����Զ�����,�����ֶ�����,���Զ����������л����˻���blockly�±༭���ݶ�ʧ
    void onSaveasProject();
    /**
     * @brief ���WEB��ľ��
     */
    void onWebClear();
protected:
    virtual void showEvent(QShowEvent* event);
    virtual void closeEvent(QCloseEvent* event);
private slots:
    /**
     * @brief WEBҳ����ؽ��ȣ��ٷ���
     */
    void onWebLoadProgress(int progress);
    /**
     * @brief WEBҳ��������
     */
    void onWebLoadFinished(bool finished);
    /**
     * @brief WEB�½�����
     */
	void onSocketNewConnection();
    /**
     * @brief �յ�WEB��Ϣ
     */
	void onSocketTextMessageReceived(QString message);
    /**
     * @brief WEB���ӶϿ�
     */
	void onSocketDisconnected();
    /**
     * @brief ѡ���豸�ı�
     * @param [in] currentName  ���豸����
     * @param [in] previousName ���豸����
     */
    void onCurrentDeviceNameChanged(QString currentName, QString previousName);
    /**
     * @brief �豸�б����豸���������
     */
    void onDeviceAdd(QString name, QString ip, float x, float y);
    /**
     * @brief �豸�б����豸�Ƴ�
     */
    void onDeviceRemove(QString name);
    /**
     * @brief �豸�б����豸������
     */
    void onDeviceRename(QString newName, QString oldName);
    /**
     * @brief �����ͨѶ
     */
    void onAppMessage(const QString& message);

private:
    /**
     * @brief �½���Ŀ�����ļ�
     * @param [in] qstrFile ��Ŀ����·������
     * @param [in] X ���ش�С
     * @param [in] Y ���ش�С
     * @return �ļ��Ƿ����ɹ�
     */
    bool newProjectFile(QString qstrFile, float X = 10.0, float Y = 10.0);
    void deviceWaypoint();
private:
    Ui::UAVManageClass ui;
    //�豸�б�
    DeviceManage* m_pDeviceManage;
    //websocket����
    QWebSocketServer* m_pSocketServer;
    //web����
    QWebSocket* m_pWebSocket;
    QString m_qstrLastWebMessage;
    //��ǰ�����ļ�
    QString m_qstrCurrentProjectFile;
    ThreadPython ptyhon;
};
