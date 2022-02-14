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
		_DeviceTakeoffLocal=1, //���
		_DeviceLandLocal,      //����
		_DeviceQuickStop,      //��ͣ
		_DeviceSetout		   //׼�����
	};
	enum _CalibrationEnum {
		_Gyro,				//����У׼
		_Magnetometer,		//������У׼ 
		_MagEnable,			//������ʹ�ܿ���
		_Remote,			//��Чֵ
		_Accelerometer,		//�Ӽ�У׼
		_Compmot,			//��Чֵ
		_Baro				//���У׼
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
	/**
	* @brief �豸У׼
	*/
	void allDeviceCalibration(_CalibrationEnum c);
	/**
	 * @brief �ϴ��貽���ɿ�
	 * @param name �豸����
	 * @param data �����б� Э����Ϣ
	 * @return ���ش�����Ϣ
	 */
	QString sendWaypoint(QString name, QVector<NavWayPointData> data);
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
	/**
	 * @brief �貽�ϴ�����
	 * @param name �豸����
	 * @param index �貽���
	 * @param count �貽����
	 * @param res �ϴ��貽��Ӧ���
	 * @param finish ���������Ƿ����
	 * @param text ��ǰ���еĹ���
	 */
	void sigWaypointProcess(QString name, unsigned int index, unsigned int count, int res, bool finish, QString text);
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);
public slots:
	void onDeviceConrolFinished(QString text, int res, QString explain);
private:
	/**
	 * @brief ��ǰѡ�е��豸
	 */
	DeviceControl* getCurrentDevice();
private:
	Ui::DeviceManage ui;
	//�豸�˵�
	QMenu* m_pMenu;
};
