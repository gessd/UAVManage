#pragma once

#include <QWidget>
#include "ui_devicecontrol.h"
#include <QMenu>
#include <QAction>
#include <QMutex>
#include <QTimer>
#include <QEvent>
#include "libhvsetting.h"
#include "mavlinksetting.h"
#include "resendmessage.h"
#include "definesetting.h"
#include "devicedebug.h"

//无人机当前姿态
struct _stDeviceCurrentStatus
{
	int x;	//厘米
	int y;	//厘米
	int z;	//厘米
	float roll;
	float pitch;
	float yaw;
	int led;
	int battery;
	_stDeviceCurrentStatus() {
		x = y = z = led = battery = 0;
		roll = pitch = yaw = 0.0;
	}
};

class DeviceControl : public QWidget
{
	Q_OBJECT
public:
	struct _NoHeartbeatStatus
	{
		//校准及上传舞步时没有心跳数据
		bool bNoHeartbeatIng;
		//关闭心跳时的UTC时间
		unsigned int nLastTime;
		_NoHeartbeatStatus() {
			bNoHeartbeatIng = false;
			nLastTime = 0;
		}
	};
public:
	DeviceControl(QString name, float x, float y, QString ip = "", QWidget *parent = Q_NULLPTR);
	~DeviceControl();
	QString getName();
	void setName(QString name);
	QString getIP();
	void setIp(QString ip);
	long getX();
	void setX(long x);
	long getY();
	void setY(long y);
	void setCurrentTime(unsigned int time);
	DeviceDebug* getDeviceDebug();
	void setStartLocation(long x, long y);
	_stDeviceCurrentStatus getCurrentStatus();
	/**
	 * @brief 允许控制
	 */
	void enableControl(bool enable);
	/**
	 * @brief 是否勾选设备
	 */
	bool isCheckDevice();
	/**
	 * @brief 设置勾选状态
	 */
	void setChcekStatus(bool check);
	/**
	 * @brief 连接设备
	 */
	bool connectDevice();

	/**
	 * @brief 断开设备连接
	 */
	void disconnectDevice();

	/**
	 * @brief 设备是否连接
	 */
	bool isConnectDevice();

	/**
	 * @brief 启用心跳
	 */
	void setHeartbeatEnable(bool enable);
	
	/**
	 * @brief 设备是否已经上传航点
	 */
	bool isUploadWaypointFinished();
	/**
	 * @brief 设备是否已经定桩授时
	 */
	bool isTimeSync();
	/**
	 * @brief 获取设备标签
	 */
	int getDeviceTag();
	/**
	 * @brief 是否处于舞步上传状态中
	 */
	bool isUploadWaypointIng();

	/**
	 * @brief 清除定桩授时状态
	 */
	void clearTimeSyncStatus();
	/**
	 * @brief 获取定桩授时的时间，便于检查设备是否在同一时间完成定桩授时
	 */
	unsigned int getTimeSyncUTC();
	/**
	 * @brief 设备是否在准备起飞状态
	 */
	bool isPrepareTakeoff();

	/**
	 * @brief 准备上传航点数据
	 * @param data 航点结构
	 * @return 返回消息错误值，0成功，根据舞步上传进度信号判断是否完成
	 */
	int DeviceMavWaypointStart(QVector<NavWayPointData> data);

	/**
	 * @brief 无人机设置模式
	 * @return 返回消息错误值 [0成功]
	 */
	int Fun_MAV_CMD_DO_SET_MODE(bool wait = true, bool again = true);

	/**
	 * @brief 无人机起飞
	 * @param Pitch        [in] 无效
	 * @param Empty        [in] 无效
	 * @param AscendRate   [in] 爬升速率
	 * @param Yaw          [in] 偏航角
	 * @param X            [in] 坐标X
	 * @param Y            [in] 坐标Y
	 * @param Z            [in] 坐标Z
	 * @param wait         [in] 阻塞等待消息返回结果
	 * @return 返回消息错误值 [0成功]
	 */
	int Fun_MAV_CMD_NAV_TAKEOFF_LOCAL(float Pitch, float Empty, float AscendRate, float Yaw, float X, float Y, float Z, bool wait = true, bool again = true);

	/**
	 * @brief 无人机降落
	 * @param Target       [in] 无效
	 * @param Offset       [in] 无效
	 * @param DescendRate  [in] 着陆下降率
	 * @param Yaw          [in] 偏航角
	 * @param X            [in] 坐标X
	 * @param Y            [in] 坐标Y
	 * @param Z            [in] 坐标Z
	 * @param wait         [in] 阻塞等待消息返回结果
	 * @return 返回消息错误值 [0成功]
	 */
	int Fun_MAV_CMD_NAV_LAND_LOCAL(float Target, float Offset, float DescendRate, float Yaw, float X, float Y, float Z, bool wait = true, bool again = true);
	
	/**
	 * @brief 无人机紧急停止
	 */
	int Fun_MAV_QUICK_STOP(bool wait = true, bool again = true);

	/**
	 * @brief 航点命令
	 * @param Hold         [in] 停留时间
	 * @param AcceptRadius [in] 接受半径
	 * @param PassRadius   [in] 轨迹控制
	 * @param Yaw          [in] 偏转角度
	 * @param Latitude     [in] 坐标X
	 * @param Longitude    [in] 坐标Y
	 * @param Altitude     [in] 坐标Z
	 * @param wait         [in] 阻塞等待消息返回结果
	 * @return 返回消息错误值 [0成功]
	 */
	int Fun_MAV_CMD_NAV_WAYPOINT(float Hold, float AcceptRadius, float PassRadius, float Yaw, float Latitude, float Longitude, float Altitude, bool wait = true, bool again = true);

	/**
     * @brief  无人机校准命令
     * @param1 陀螺校准
     * @param2 磁罗盘校准
     * @param3 磁罗盘开关 [开关为状态值，设置后保存，其他为单次使用值，使用后无效]
     * @param4 未使用
     * @param5 加计校准
     * @param6 未使用
     * @param7 电调校准
     */
	int Fun_MAV_CALIBRATION(float p1, float p2, float p3, float p4, float p5, float p6, float p7, bool wait = true, bool again = true);

	/**
	 * @brief  无人机列队
	 */
	int Fun_MAV_Defined_Queue(int x, int y);
	
	/**
	 * @brief 无人机回收
	 */
	int Fun_MAV_Defined_Regain(int x, int y);

	/**
	 * @brief 无人机灯光
	 */
	int Fun_MAV_LED_MODE();

	/**
	 * @brief 定桩授时
	 */
	int Fun_MAV_TimeSync();
private slots:
	/**
	* @brief 发送数据
	* @param 是否为重发消息
	* @param data [in] 二进制数据
	* @return 发送成功
	*/
	bool sendMessage(bool again, QByteArray data);
	/**
	 * @brief 更新界面电量
	 * @param 电压
	 * @param 电流
	 * @param 剩余电量，百分比
	 */
	void onUpdateBatteryStatus(float voltages, float battery, unsigned short electric);

	/**
	 * @brief 更新界面定位
	 * @param 时间
	 * @param X 厘米
	 * @param Y 厘米
	 * @param Z 厘米
	 */
	void onUpdateLocation(unsigned int time, int x, int y, int z);
private:
	/**
	* @brief TCP连接状态，子线程回调函数
	* @param SocketChannelPtr TCP通道
	*/
	void hvcbConnectionStatus(const hv::SocketChannelPtr& channel);
	/**
	* @brief TCP接收到的消息，子线程回调函数
	* @param SocketChannelPtr TCP通道
	* @param Buffer 数据内容,注意数据长度
	*/
	void hvcbReceiveMessage(const hv::SocketChannelPtr& channel, hv::Buffer* buf);
	/**
	* @brief TCP已发送的数据，子线程回调函数
	* @param SocketChannelPtr TCP通道
	* @param Buffer 数据内容,注意数据长度
	*/
	void hvcbWriteComplete(const hv::SocketChannelPtr& channel, hv::Buffer* buf);
	/**
	* @brief mavlink消息结构转为字符串
	* @param mavlink_message_t [in] mavlink消息结构
	* @return 字符串数据
	*/
	QByteArray mavMessageToBuffer(mavlink_message_t mesage);
	QByteArray mavCommandLongToBuffer(float param1, float param2, float param3, float param4, float param5, float param6, float param7, int command, int confirmation = 1);
private:
	/**
	 * @brief 发送Mav指令消息
	 * @param qstrText      指令名称
	 * @param commandID     指令ID
	 * @param arrData       发送的数据内容
	 * @param arrAgainData  重发的数据内容，部分协议重发数据与初始数据不同
	 * @param bWait         等待发送结果
	 * @param againNum      重发次数
	 * @param againInterval 重发/超时时间间隔[毫秒]
	 * @return 返回执行结果
	 */
	int MavSendCommandLongMessage(QString name, int commandID, QByteArray arrData, QByteArray arrAgainData="", bool bWait = false
		, unsigned int againNum = _MavLinkResendNum_, unsigned int againInterval = _NkCommandResendInterval_);
	QByteArray getWaypointData(float param1, float param2, float param3, float param4
		, int32_t x, int32_t y, float z, uint16_t seq, unsigned int commandID, unsigned int again);
	/**
	 * @brief 发送航点结束指令
	 */
	void DeviceMavWaypointEnd(unsigned int count);
	/**
	 * @brief 心跳响应更新
	 */
	void onUpdateHeartBeat();
	/**
	 * @brief 更新设备连接状态
	 */
	void onUpdateConnectStatus(QString name, QString ip, bool connect);
	/**
	 * @brief 更新Tip框内容
	 */
	void updateToopTip();
signals:
	/**
	* @brief 设备命令控制返回结果
	* 
	* @param 指令名称
	* @param 消息返回结果
	* @param 消息错误说明
	*/
	void sigConrolFinished(int nCommanId, QString text, int res, QString explain);
	/**
	 * @brief 心跳响应更新
	 */
	void sigUpdateHeartbeat();
	/**
	 * @brief COMMAND消息返回值
	 * @param 设备名称
	 * @param 设备IP地址
	 * @param 连接成功与断开
	 */
	void sigConnectStatus(QString name, QString ip, bool connect);
	/**
	 * @brief COMMAND消息返回值
	 * @param 设备名称
	 * @param result    返回值
	 * @param commandid 指令ID
	 */
	void sigCommandResult(QString name, int result, int commandid);
	/**
	 * @brief 舞步上传完成
	 * @param 无人机名称
	 * @param 成功/失败
	 * @param 说明
	 */
	void sigWaypointFinished(QString name, bool success, QString text);
	/**
	 * @brief 电池信息
	 * @param 电压
	 * @param 电流
	 * @param 剩余电量，百分比
	 */
	void sigBatteryStatus(float voltages, float battery, unsigned short electric);
	//通讯数据 [true接收|flase发送]
    void sigMessageByte(QByteArray, bool, int);
	//日志消息
	void sigLogMessage(QString data);
	//当前位置信息
	void sigLocalPosition(unsigned int time_boot_ms, int x, int y, int z);
	//无人机标签
	void sigUpdateTag(unsigned int n);
	//IMU数据
	void sigHighresImu(unsigned long long, QList<float>);
	//姿态角
	void sigAttitude(unsigned int time, float roll, float pitch, float yaw);
	/*
	* @brief 设备设备
	* @param 设备名称
	*/
	void sigRemoveDevice(QString name);
private slots:
	void onWaypointStart();
	void onWaypointNext();
	void onMessageThreadFinished();
private:
	Ui::DeviceControl ui;
	QString m_qstrIP;
	//需要使用指针,stop后需要释放指针否则再次createsocket时因loop指针为空会出现崩溃
	hv::TcpClient* m_pHvTcpClient;
	QMutex m_mutexMavMessageToBurrer;
	bool m_bHeartbeatEnable;
	long m_nStartX;
	long m_nStartY;
	//航点下发中
	bool m_bWaypointSending;
	//航点数据
	QVector<NavWayPointData> m_currentWaypointData;
	//当前上传航点进度
	int m_nCurrentWaypontIndex;
	//调试窗口s
	DeviceDebug* m_pDebugDialog;
	//检测心跳定时器
	QTimer m_timerHeartbeat;
	//校准操作时不判断心跳超时
	_NoHeartbeatStatus m_heartbeatStatus;
	//设备状态 电量位置及姿态
	_stDeviceCurrentStatus m_deviceStatus;
	//当前音乐播放进度
	unsigned int m_nCurrentMusicTime;
	//已上传航点
	bool m_bUploadFinished;
	//已定桩授时
	bool m_bTimeSync;
	unsigned int m_nTimeSynsUTC;
	//是否在准备起飞状态
	bool m_bPrepareTakeoff;
	//mavlink解包参数，区分不同设备数据
	int m_nMavChan;
	//无人机标签 不可重复，UWB基站定位使用，负值则无效
	int m_nUWBTag;
};
