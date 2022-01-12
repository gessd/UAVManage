#pragma once

#include <QWidget>
#include "ui_devicecontrol.h"
#include <QMenu>
#include <QAction>
#include <QMutex>
#include "libhvsetting.h"
#include "mavlinksetting.h"
#include "resendmessage.h"

class DeviceControl : public QWidget
{
	Q_OBJECT
public:
	enum _DeviceStatus 
	{
		DeviceUnConnect = -1,
		DeviceMessageToimeout = -404
	};
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
	 * @brief 发送数据
	 * @param data [in] 二进制数据
	 * @return 发送成功
	 */
	bool sendMessage(QByteArray data);
	/**
	 * @brief 启用心跳
	 */
	void setHeartbeatEnable(bool enable);

	/**
	 * @brief 无人机设置模式
	 * @param Mode        [in] 模式类型 1姿态模式|2定高模式|3航点模式
	 * @return 返回消息错误值 [0成功]
	 */
	int Fun_MAV_CMD_DO_SET_MODE(float Mode, bool wait = false);

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
	int Fun_MAV_CMD_NAV_TAKEOFF_LOCAL(float Pitch , float Empty, float AscendRate, float Yaw, float X, float Y, float Z, bool wait=false);

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
	int Fun_MAV_CMD_NAV_LAND_LOCAL(float Target, float Offset, float DescendRate, float Yaw, float X, float Y, float Z, bool wait = false);
	
	/**
	 * @brief 无人机紧急停止
	 */
	int Fun_MAV_QUICK_STOP();

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
	int Fun_MAV_CMD_NAV_WAYPOINT(float Hold, float AcceptRadius, float PassRadius, float Yaw, float Latitude, float Longitude, float Altitude, bool wait = false);

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
	int Fun_MAV_CALIBRATION(float p1, float p2, float p3, float p4, float p5, float p6, float p7, bool wait = false);

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
	int sendNavCommandLong(float param1, float param2, float param3, float param4, float param5, float param6, float param7, int command, bool wait);
signals:
	void sigConnectStatus(QString, bool);
	/**
	 * @brief COMMAND消息返回值
	 * @param result    返回值
	 * @param commandid 指令ID
	 */
	void sigCommandResult(int result, int commandid);
private:
	Ui::DeviceControl ui;
	QString m_qstrIP;
	//需要使用指针,stop后需要释放指针否则再次createsocket时因loop指针为空会出现崩溃
	hv::TcpClient* m_pHvTcpClient;
	QMutex m_mutexMavMessageToBurrer;
	bool m_bHeartbeatEnable;
	float m_fStartX;
	float m_fStartY;
};
