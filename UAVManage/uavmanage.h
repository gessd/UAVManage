#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_uavmanage.h"
#include <QDebug>
#include <QProcess>
#include <QWebSocket>
#include <QWebSocketServer>
//python�궨����mavlink�к궨���г�ͻ��python���ñ���ŵ�mavlink����֮ǰ
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
    * @brief �½���Ŀ
    */
    void onNewProject();
    /**
    * @brief ����Ŀ
    */
    void onOpenProject(QString qstrFile);
    //ÿһ�����Զ�����,�����ֶ�����,���Զ����������л����˻���blockly�±༭���ݶ�ʧ
    /**
    * @brief ��Ŀ���Ϊ
    */
    void onSaveasProject();
    /**
     * @brief ���WEB��ľ��
     */
    void onWebClear();
protected:
    virtual void showEvent(QShowEvent* event);
    virtual void closeEvent(QCloseEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
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
    * @brief �豸���ָ���·����
    * @param ���|����
    */
    void onDeviceTakeoffFinished(bool takeoff);
    /**
     * @brief �����ͨѶ
     */
    void onAppMessage(const QString& message);
    /**
     * @brief �����ϴ�����
     */
    void onWaypointProcess(QString name, unsigned int index, unsigned int count, int res, bool finish, QString text);
    /**
    * @brief ���¼��������ļ�
    */
    void onUpdateMusic(QString qstrFilePath);
    /**
    * @brief ��ǰ���ֲ��Ž���
    */
    void onCurrentMusicTime(int second);
    /**
    * @brief ��ǰ���ֲ���״̬
    */
    void onCurrentPlayeState(qint8 state);
    /**
    * @brief ��ά��������״̬
    */
    void on3DDialogStauts(bool connect);
private:
    /**
     * @brief �½���Ŀ�����ļ�
     * @param [in] qstrFile ��Ŀ����·������
     * @param [in] X ���ش�С
     * @param [in] Y ���ش�С
     * @return �ļ��Ƿ����ɹ�
     */
    bool newProjectFile(QString qstrFile, float X = 10.0, float Y = 10.0);
    /**
     * @brief �����貽
     * @param bUpload �Ƿ��ϴ��貽���豸
     */
    void deviceWaypoint(bool bUpload = false);
private:
    Ui::UAVManageClass ui;
    //�豸�б�
    DeviceManage* m_pDeviceManage;
    //websocket���� ��ľ�����
    QWebSocketServer* m_pSocketServer;
    //web���� ��ľ�����
    QWebSocket* m_pWebSocket;
    QString m_qstrLastWebMessage;
    //��ǰ�����ļ�
    QString m_qstrCurrentProjectFile;
    ThreadPython ptyhon;
    SoundGrade* m_pSoundWidget;
    QProcess* m_p3DProcess;
};
