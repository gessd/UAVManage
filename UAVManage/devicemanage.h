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
	* @brief 添加设备
	* @param [in] qstrName设备名称
	* @param [in] ip设备IP
	* @return 添加成功或失败
	*/
	bool addDevice(QString qstrName, QString ip);
	/**
	* @brief 删除设备
	*/
	void removeDevice();
	/**
	 * @brief 清空所有设备
	 */
	void clearDevice();
	/**
	 * @brief 获取当前设备名
	 */
	QString getCurrentDeviceName();
	/**
	* @brief 获取新的可用的设备名称,因设备名称不可重复
	* @return 按顺序返回设备名称
	*/
	QString getNewDefaultName();
	/**
	* @brief 判断名称是否重复
	* @param [in] qstrName 设备名
	* @return 如果和已存在的设备名称重复返回TRUE
	*/
	bool isRepetitionName(QString qstrName);
signals:
	void currentDeviceNameChanged(QString currentName, QString previousName);
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);
private:
	DeviceControl* getCurrentDevice();
private:
	Ui::DeviceManage ui;
	//已记录的设备，名称及IP不可以重复
	//QMap<QString, _tagDeviceProperty> m_mapDevices;
	//设备菜单
	QMenu* m_pMenu;
};
