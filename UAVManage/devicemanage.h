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
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "definesetting.h"

class DeviceManage : public QWidget
{
	Q_OBJECT

	//Q_PROPERTY(QString deviceName READ getCurrentName WRITE setCurrentName)
public:
	enum _AllDeviceCommand {
		_DeviceTakeoffLocal=1, //起飞
		_DeviceLandLocal,      //降落
		_DeviceQuickStop,      //急停
		_DeviceSetout		   //准备起飞
	};
	enum _CalibrationEnum {
		_Gyro,				//陀螺校准
		_Magnetometer,		//磁罗盘校准 
		_MagEnable,			//磁罗盘使能开关
		_Remote,			//无效值
		_Accelerometer,		//加计校准
		_Compmot,			//无效值
		_Baro				//电调校准
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
	* @brief 判断是否重复,为空则不判断
	* @param [in] qstrName 设备名
	* @param [in] ip 设备IP
	* @param [in] 坐标X
	* @param [in] 坐标Y
	* @param [in] 是否判断坐标重复
	* @return 如果设备属于有冲突则返回TRUE
	*/
	QString isRepetitionDevice(QString qstrName, QString ip, float x, float y, bool location = false);
	/**
	 * @brief 控制所有设备执行
	 */
	void allDeviceControl(_AllDeviceCommand comand);
	/**
	* @brief 设备校准
	*/
	void allDeviceCalibration(_CalibrationEnum c);
	/**
	 * @brief 上传舞步到飞控/三维舞步更新
	 * @param name 设备名称
	 * @param data 航点列表 协议信息
	 * @param upload 是否上传舞步到飞控
	 * @return 返回错误信息
	 */
	QString sendWaypoint(QString name, QVector<NavWayPointData> data, bool upload);
	/**
	* @brief 根据音乐进度更新舞步时间
	* @param 秒值
	*/
	void setUpdateWaypointTime(int second);
	/**
	* @brief 设置当前音频播放状态
	*/
	void setCurrentPlayeState(qint8 state);
	/**
	* @brief 三维设置音乐文件路径
	*/
	void setCurrentMusicPath(QString filePath);
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
	 * @brief 设备修改IP地址
	 * @param 设备名
	 * @param IP地址
	 */
	void deviceResetIp(QString qstrName, QString ip);
	/**
	 * @brief 当前选中设备改变
	 * @param 当前选中设备
	 * @param 上一次选中设备，可能存在空
	 */
	void currentDeviceNameChanged(QString currentName, QString previousName);
	/**
	 * @brief 舞步上传进度
	 * @param name 设备名称
	 * @param index 舞步序号
	 * @param count 舞步总数
	 * @param res 上传舞步响应结果
	 * @param finish 整个过程是否完成
	 * @param text 当前进行的过程
	 */
	void sigWaypointProcess(QString name, unsigned int index, unsigned int count, int res, bool finish, QString text);
	/**
	* @brief 设备起飞指令下发完成
	* @param 起飞|降落
	*/
	void sigTakeoffFinished(bool takeoff);
	/**
	* @brief 三维窗口连接状态
	*/
	void sig3DDialogStatus(bool connect);
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);
private slots:
	void onDeviceConrolFinished(QString text, int res, QString explain);
	void on3dNewConnection();
private:
	/**
	 * @brief 当前选中的设备
	 */
	DeviceControl* getCurrentDevice();
private:
	Ui::DeviceManage ui;
	//设备菜单
	QMenu* m_pMenu;
	//三维模拟通讯使用
	QTcpServer* m_p3dTcpServer;
	QTcpSocket* m_p3dTcpSocket;
};
