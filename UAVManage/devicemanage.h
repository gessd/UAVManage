#pragma once

#include <QWidget>
#include "ui_devicemanage.h"
#include "adddevicedialog.h"
#include "devicecontrol.h"
#include <QMenu>
#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextCodec>
#include <QMessageBox>
#include "definesetting.h"

class DeviceManage : public QWidget
{
	Q_OBJECT

	//Q_PROPERTY(QString deviceName READ getCurrentName WRITE setCurrentName)
public:
	enum _AllDeviceCommand {
		_DeviceTakeoffLocal, //���
		_DeviceLandLocal,    //����
		_DeviceQuickStop     //��ͣ
	};
	DeviceManage(QWidget *parent = Q_NULLPTR);
	~DeviceManage();
	/**
	* @brief ����豸
	* @param [in] qstrName �豸����
	* @param [in] ip �豸IP
	* @param [in] x ��ʼλ��
	* @param [in] y ��ʼλ��
	* @return ������Ϣ,�ɹ�Ϊ��
	*/
	QString addDevice(QString qstrName, QString ip, float x, float y);
	/**
	* @brief ɾ���豸
	*/
	void removeDevice();
	/**
	 * @brief ��������豸
	 */
	void clearDevice();
	/**
	 * @brief ��ȡ��ǰ�豸��
	 */
	QString getCurrentDeviceName();
	/**
	 * @brief ���õ�ǰѡ���豸
	 */
	bool setCurrentDevice(QString qstrName);
	/**
	 * @brief ��ȡ�豸�����б�,�����б�˳��
	 */
	QStringList getDeviceNameList();
	/**
	* @brief ��ȡ�µĿ��õ��豸����,���豸���Ʋ����ظ�
	* @return ��˳�򷵻��豸����
	*/
	QString getNewDefaultName();
	/**
	* @brief �ж������Ƿ��ظ�
	* @param [in] qstrName �豸��
	* @return ������Ѵ��ڵ��豸�����ظ�����TRUE
	*/
	bool isRepetitionName(QString qstrName);
	/**
	 * @brief ���������豸ִ��
	 */
	void allDeviceControl(_AllDeviceCommand comand);
signals:
	/**
	 * @brief �豸������
	 * @param �豸��
	 * @param IP
	 * @param ��ʼ����
	 * @param ��ʼ����
	 */
	void deviceAddFinished(QString qstrName, QString ip, float x, float y);
	/**
	 * @brief �豸ɾ�����
	 * @param �豸��
	 */
	void deviceRemoveFinished(QString qstrName);
	/**
	 * @brief �豸���������
	 * @param ���豸��
	 * @param ���豸��
	 */
	void deviceRenameFinished(QString newName, QString oldName);
	/**
	 * @brief ��ǰѡ���豸�ı�
	 * @param ��ǰѡ���豸
	 * @param ��һ��ѡ���豸�����ܴ��ڿ�
	 */
	void currentDeviceNameChanged(QString currentName, QString previousName);
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);
private slots:
	
private:
	/**
	 * @brief ��ǰѡ�е��豸
	 */
	DeviceControl* getCurrentDevice();
private:
	Ui::DeviceManage ui;
	//�Ѽ�¼���豸�����Ƽ�IP�������ظ�
	//QMap<QString, _tagDeviceProperty> m_mapDevices;
	//�豸�˵�
	QMenu* m_pMenu;
};
