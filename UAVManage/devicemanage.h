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
		_DeviceTakeoffLocal, //起飞
		_DeviceLandLocal,    //降落
		_DeviceQuickStop     //急停
	};
	DeviceManage(QWidget *parent = Q_NULLPTR);
	~DeviceManage();
	/**
	* @brief 添加设备
	* @param [in] qstrName 设备名称
	* @param [in] ip 设备IP
	* @param [in] x 初始位置
	* @param [in] y 初始位置
	* @return 错误信息,成功为空
	*/
	QString addDevice(QString qstrName, QString ip, float x, float y);
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
	 * @brief 设置当前选中设备
	 */
	bool setCurrentDevice(QString qstrName);
	/**
	 * @brief 获取设备名称列表,按照列表顺序
	 */
	QStringList getDeviceNameList();
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
	/**
	 * @brief 控制所有设备执行
	 */
	void allDeviceControl(_AllDeviceCommand comand);
signals:
	/**
	 * @brief 设备添加完成
	 * @param 设备名
	 * @param IP
	 * @param 初始坐标
	 * @param 初始坐标
	 */
	void deviceAddFinished(QString qstrName, QString ip, float x, float y);
	/**
	 * @brief 设备删除完成
	 * @param 设备名
	 */
	void deviceRemoveFinished(QString qstrName);
	/**
	 * @brief 设备重命名完成
	 * @param 新设备名
	 * @param 旧设备名
	 */
	void deviceRenameFinished(QString newName, QString oldName);
	/**
	 * @brief 当前选中设备改变
	 * @param 当前选中设备
	 * @param 上一次选中设备，可能存在空
	 */
	void currentDeviceNameChanged(QString currentName, QString previousName);
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);
private slots:
	
private:
	/**
	 * @brief 当前选中的设备
	 */
	DeviceControl* getCurrentDevice();
private:
	Ui::DeviceManage ui;
	//已记录的设备，名称及IP不可以重复
	//QMap<QString, _tagDeviceProperty> m_mapDevices;
	//设备菜单
	QMenu* m_pMenu;
};
