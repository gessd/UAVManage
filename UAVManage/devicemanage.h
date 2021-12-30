#pragma once

#include <QWidget>
#include "ui_devicemanage.h"
#include "adddevicedialog.h"
#include "devicecontrol.h"
#include <QMenu>
#include <QAction>

class DeviceManage : public QWidget
{
	Q_OBJECT

	//Q_PROPERTY(QString deviceName READ getCurrentName WRITE setCurrentName)
public:
	DeviceManage(QWidget *parent = Q_NULLPTR);
	~DeviceManage();
	/**
	* @brief ����豸
	* @param [in] qstrName�豸����
	* @param [in] ip�豸IP
	* @return ��ӳɹ���ʧ��
	*/
	bool addDevice(QString qstrName, QString ip);
	/**
	* @brief ɾ���豸
	*/
	void removeDevice();
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
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);
private:
	DeviceControl* getCurrentDevice();
private:
	Ui::DeviceManage ui;
	QStringList m_listDeviceNames;
	QMenu* m_pMenu;
};
