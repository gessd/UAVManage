#pragma once

#include <QWidget>
#include "ui_devicemanage.h"
//python宏定义与mavlink中宏定义有冲突，python引用必须放到mavlink引用之前
#include "threadpython.h"
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
		_DeviceSetout,		   //准备起飞
		_DeviceQueue,			//列队
		_DeviceRegain			//回收
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
	 * @brief 设定场地大小
	 */
	void setSpaceSize(unsigned int x, unsigned int y);

	/**
	 * @brief 获取场地大小
	 */
	QPoint getSpaceSize();

	/**
	 * @brief 设置基站位置
	 */
	void setStationAddress(QMap<QString, QPoint> station);

	/**
	* @brief 添加设备
	* @param [in] qstrName 设备名称
	* @param [in] ip 设备IP
	* @param [in] x 初始位置
	* @param [in] y 初始位置
	* @return 错误信息,成功为空
	*/
	QString addDevice(QString qstrName, QString ip, long x, long y);
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
	*/
	QString isRepetitionDevice(QString qstrName, QString ip, long x, long y, QString type);
	/**
	 * @brief 控制所有设备执行
	 */
	void allDeviceControl(_AllDeviceCommand comand);
	/**
	* @brief 设备校准
	*/
	void allDeviceCalibration(_CalibrationEnum c);

	/**
	 * @brief 生成并上传舞步
	 * @param 打开的工程文件，根据工程文件目录打开每个无人机的py文件
	 * @param 是否上传到飞控
	 */
	void waypointComposeAndUpload(QString qstrProjectFile, bool upload);

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
	void setCurrentMusicPath(QString filePath, QPixmap pixmap);
	/**
	 * @brief 发送航点列表到三维
	 * @param 所有设备航点,根据名称区分
	 * 因设备航点是每个设备单独发送，故发的三维的航点与设备分开
	 */
	void sendWaypointTo3D(QMap<QString, QVector<NavWayPointData>> map);
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
	 * @brief 设备初始位置被修改
	 * @param 设备名
	 */
	void deviceResetLocation(QString name, long x, long y);
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
	/**
	 * @brief 三维窗口建立连接
	 */
	void on3dNewConnection();
	/**
	 * @brief 定时处理三维消息发送记录
	 */
	void onTimeout3DMessage();
	/**
	 * @brief 定时发送设备状态到三维
	 */
	void onUpdateStatusTo3D();
private:
	/**
	 * @brief 当前选中的设备
	 */
	DeviceControl* getCurrentDevice();
	/**
	 * @brief 根据设备名称获取设备控制对象
	 */
	DeviceControl* getDeviceFromName(QString name);
	/**
	 * @brief 通过TCP向三维发送消息
	 */
	void sendMessageTo3D(QJsonObject json3d);
	/**
	 * @@brief 处理接收的三维消息
	 */
	void analyzeMessageFrom3D(QByteArray data);
	/**
	 * @@brief 计算两点直接距离
	 */
	int getDistance(int x1, int y1, int z1, int x2, int y2, int z2);
private:
	Ui::DeviceManage ui;
	//设备菜单
	QMenu* m_pMenu;
	//三维模拟通讯使用
	QTcpServer* m_p3dTcpServer;
	QTcpSocket* m_p3dTcpSocket;
	//三维消息记录，用于发送失败重发
	QMap<int, QJsonObject> m_map3DMsgRecord;
	QTimer m_timerMessage3D;
	//定时发送无人机姿态数据到三维
	QTimer m_timerUpdateStatus;
	//用于执行python代码
	ThreadPython pythonThread;
	//场地大小
	QPoint m_pointSpace;
	//基站定位位置 基站名称及基站位置
	QMap<QString, QPoint> m_stationMap;
};
