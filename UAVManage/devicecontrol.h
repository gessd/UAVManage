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

class DeviceControl : public QWidget
{
	Q_OBJECT
public:
	
public:
	DeviceControl(QString name, float x, float y, QString ip = "", QWidget *parent = Q_NULLPTR);
	~DeviceControl();
	QString getName();
	void setName(QString name);
	QString getIP();
	void setIp(QString ip);
	float getX();
	void setX(float x);
	float getY();
	void setY(float y);
	DeviceDebug* getDeviceDebug();
	void setStartLocation(float x, float y);
	QList<float> getStartLocation();

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
	 * @brief 准备上传航点数据，根据信号sigWaypointProcess处理进度
	 * @param data 航点结构
	 * @return 返回消息错误值，0成功，根据舞步上传进度信号判断是否完成
	 */
	int DeviceMavWaypointStart(QVector<NavWayPointData> data);

	/**
	 * @brief 无人机设置模式
	 * @param Mode        [in] 模式类型 1姿态模式|2定高模式|3航点模式
	 * @return 返回消息错误值 [0成功]
	 */
	int Fun_MAV_CMD_DO_SET_MODE(float Mode, bool wait = true, bool again = true);

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
	int Fun_MAV_Defined_Queue();
	
	/**
	 * @brief 无人机回收
	 */
	int Fun_MAV_Defined_Regain();

	/**
	 * @brief 无人机灯光
	 */
	int Fun_MAV_LED_MODE();
private slots:
	/**
	* @brief 发送数据
	* @param data [in] 二进制数据
	* @return 发送成功
	*/
	bool sendMessage(QByteArray data);
	/**
	 * @brief 更新界面电量
	 * @param 电压
	 * @param 电流
	 * @param 剩余电量，百分比
	 */
	void onUpdateBatteryStatus(float voltages, float battery, unsigned short electric);
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
		, int32_t x, int32_t y, float z, uint16_t seq, unsigned int again);
	/**
	 * @brief 开始发送航点
	 */
	void DeviceMavWaypointSend(QVector<NavWayPointData> data);
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
signals:
	/**
	* @brief 设备命令控制返回结果
	* @param 指令名称
	* @param 消息返回结果
	* @param 消息错误说明
	*/
	void sigConrolFinished(QString text, int res, QString explain);
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
	 * @brief 航点下发进度
	 * @param 设备名称
	 * @param 当前航点序号
	 * @param 航点总数
	 * @param 下发过程中指令响应值
	 * @param 整个下发过程是否完成，完成并不代表成功[0=res&&true=finish]才成功
	 * @param 文字描述信息
	 */
	void sigWaypointProcess(QString name, unsigned int index, unsigned int count, int res, bool finish, QString text);
	/**
	 * @brief 电池信息
	 * @param 电压
	 * @param 电流
	 * @param 剩余电量，百分比
	 */
	void sigBatteryStatus(float voltages, float battery, unsigned short electric);
	//已发送的的指令
    void sigMessageByte(QByteArray, bool);
	//日志消息
	void sigLogMessage(QByteArray data);
	//当前位置信息
	void sigLocalPosition(unsigned int time_boot_ms, float x, float y, float z);
	//IMU数据
	void sigHighresImu(unsigned long long, QList<float>);
	//姿态角
	void sigAttitude(unsigned int time, float roll, float pitch, float yaw);
private slots:
	void onWaypointProcess(QString name, unsigned int index, unsigned int count, int res, bool finish, QString text);
	void onWaypointNext();
	void onMessageThreadFinished();
private:
	Ui::DeviceControl ui;
	QString m_qstrIP;
	//需要使用指针,stop后需要释放指针否则再次createsocket时因loop指针为空会出现崩溃
	hv::TcpClient* m_pHvTcpClient;
	QMutex m_mutexMavMessageToBurrer;
	bool m_bHeartbeatEnable;
	float m_fStartX;
	float m_fStartY;
	//航点下发中
	bool m_bWaypointSending;
	QVector<NavWayPointData> m_currentWaypointData;
	DeviceDebug* m_pDebugDialog;
	//检测心跳定时器
	QTimer m_timerHeartbeat;
};
