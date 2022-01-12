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
    //ÿһ�����Զ�����,�����ֶ�����,���Զ����������л����˻���blockly�±༭���ݶ�ʧ
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
    /**
     * @brief �½���Ŀ�����ļ�
     * @param [in] qstrFile ��Ŀ����·������
     * @param [in] X ���ش�С
     * @param [in] Y ���ش�С
     * @return �ļ��Ƿ����ɹ�
     */
    bool newProjectFile(QString qstrFile, float X = 10.0, float Y = 10.0);
private:
    Ui::UAVManageClass ui;
    //�豸�б�
    DeviceManage* m_pDeviceManage;
    //websocket����
    QWebSocketServer* m_pSocketServer;
    //web����
    QWebSocket* m_pWebSocket;
};
