#include "devicecontrol.h"
#include <QDebug>
#include <QTime>
#include <QDateTime>
#include "messagelistdialog.h"

#define _CurrentTime_  QTime::currentTime().toString("hh:mm:ss.zzz")

DeviceControl::DeviceControl(QString name, float x, float y, QString ip, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_pHvTcpClient = new hv::TcpClient;
	m_bHeartbeatEnable = true;
	m_bWaypointSending = false;
	ui.progressBar->setVisible(false);
	ui.btnSet->setVisible(false);
	QPixmap pixmap(":/res/images/uavred.png");
	ui.labelStatus->setPixmap(pixmap.scaled(ui.labelStatus->size()));
	//发送消息为了转移到主线程中处理，需要对界面进行处理
	connect(this, &DeviceControl::sigWaypointProcess, this, &DeviceControl::onWaypointProcess);
	connect(this, &DeviceControl::sigBatteryStatus, this, &DeviceControl::onUpdateBatteryStatus);
	connect(this, &DeviceControl::sigConnectStatus, this, &DeviceControl::onUpdateConnectStatus);
	connect(this, &DeviceControl::sigUpdateHeartbeat, this, &DeviceControl::onUpdateHeartBeat);
	
	qRegisterMetaType<QList<float>>("QList<float>");
	m_pDebugDialog = new DeviceDebug(ip, name, this);
	connect(this, &DeviceControl::sigConnectStatus, m_pDebugDialog, &DeviceDebug::onConnectStatus);
	connect(this, &DeviceControl::sigMessageByte, m_pDebugDialog, &DeviceDebug::onDeviceMessage);
	connect(this, &DeviceControl::sigHighresImu, m_pDebugDialog, &DeviceDebug::onUpdateHighresImu);
	connect(this, &DeviceControl::sigAttitude, m_pDebugDialog, &DeviceDebug::onUpdateAttitude);
	connect(this, &DeviceControl::sigLogMessage, m_pDebugDialog, &DeviceDebug::onMessageData);
	connect(this, &DeviceControl::sigLocalPosition, m_pDebugDialog, &DeviceDebug::onUpdateLocalPosition);

	connect(&m_timerHeartbeat, &QTimer::timeout, [this]() {
		unsigned int nLastTime = m_timerHeartbeat.property("time").toUInt();
		if ((QDateTime::currentDateTime().toTime_t() - nLastTime) > 10) {
			//TODO 超过N次没有收到心跳数据断开重新连接
			//connectDevice();
		}
		});
	setName(name);
	setIp(ip);
	setX(x);
	setY(y);
}

DeviceControl::~DeviceControl()
{
	if (m_pDebugDialog) {
		m_pDebugDialog->close();
		delete m_pDebugDialog;
		m_pDebugDialog = nullptr;
	}
	disconnectDevice();
}

QString DeviceControl::getName()
{
	return ui.labelDeviceName->text().trimmed();
}

void DeviceControl::setName(QString name)
{
	ui.labelDeviceName->setText(name);
}

QString DeviceControl::getIP()
{
	return m_qstrIP;
}

void DeviceControl::setIp(QString ip)
{
	//此处会连接设备,如果无法连接则耗时比较长
	if (m_qstrIP == ip) return;
	m_qstrIP = ip;
	disconnectDevice();
	if (m_qstrIP.isEmpty()) return;
	//连接设备
	connectDevice();
}

long DeviceControl::getX()
{
	return m_nStartX;
}

void DeviceControl::setX(long x)
{
	m_nStartX = x;
}

long DeviceControl::getY()
{
	return m_nStartY;
}

void DeviceControl::setY(long y)
{
	m_nStartY = y;
}

DeviceDebug* DeviceControl::getDeviceDebug()
{
	return m_pDebugDialog;
}

void DeviceControl::setStartLocation(long x, long y)
{
	setX(x);
	setY(y);
}

QList<long> DeviceControl::getStartLocation()
{
	QList<long> list;
	list << getX() << getY();
	return list;
}

_stDeviceCurrentStatus DeviceControl::getCurrentStatus()
{
	return m_deviceStatus;
}

//连接设备
bool DeviceControl::connectDevice()
{
	//无论是否连接，需要先把上一次的tcp对象删除后重建，否则造成崩溃
	disconnectDevice();
	if (nullptr == m_pHvTcpClient) m_pHvTcpClient = new hv::TcpClient;
	if (nullptr == m_pHvTcpClient) return false;
	int connfd = m_pHvTcpClient->createsocket(_DevicePort_, m_qstrIP.toLatin1());
	if (connfd < 0)  return false;

	//绑定回调函数
	m_pHvTcpClient->onConnection = std::bind(&DeviceControl::hvcbConnectionStatus, this, std::placeholders::_1);
	m_pHvTcpClient->onMessage = std::bind(&DeviceControl::hvcbReceiveMessage, this, std::placeholders::_1, std::placeholders::_2);
	m_pHvTcpClient->onWriteComplete = std::bind(&DeviceControl::hvcbWriteComplete, this, std::placeholders::_1, std::placeholders::_2);

	//配置自动重连模式
	hv::ReconnectInfo reconn;
	reconn.min_delay = 1000;
	reconn.max_delay = 10000;
	reconn.delay_policy = 2;
	m_pHvTcpClient->setReconnect(&reconn);
	m_pHvTcpClient->start();
	return true;
}

//断开连接
void DeviceControl::disconnectDevice()
{
	if (isConnectDevice()) 
		m_pHvTcpClient->stop();
	SAFE_DELETE(m_pHvTcpClient);
}

//是否已连接
bool DeviceControl::isConnectDevice()
{
	if (nullptr == m_pHvTcpClient) return false;
	return m_pHvTcpClient->isConnected();
}

//发送数据
bool DeviceControl::sendMessage(QByteArray data)
{
	if (data.isEmpty()) return false;
	if (!isConnectDevice()) return false;
	int len = m_pHvTcpClient->send(data.data(), data.length());
	if (len != data.length()) return false;
	return true;
}

//无人机设置是否启用心跳
void DeviceControl::setHeartbeatEnable(bool enable)
{
	m_bHeartbeatEnable = enable;
}

//无人机设置模式
int DeviceControl::Fun_MAV_CMD_DO_SET_MODE(float Mode, bool wait, bool again)
{
	//参数2,3,4,5,6,7无效
	//1姿态模式|2定高模式|3航点模式
	QByteArray arrData = mavCommandLongToBuffer(Mode, 0, 0, 0, 0, 0, 0, MAV_CMD_DO_SET_MODE);
	return MavSendCommandLongMessage(tr("准备起飞"), MAV_CMD_DO_SET_MODE, arrData, again ? arrData : "", false);
}

int DeviceControl::DeviceMavWaypointStart(QVector<NavWayPointData> data)
{
	qDebug() << "准备发送航点" << ui.labelDeviceName->text() << _CurrentTime_;
	if (!isConnectDevice()) return DeviceUnConnect;
	int count = data.count();
	if (count <= 0) return DeviceDataError;

	//标记航点下发是否进行中
	if (m_bWaypointSending) return DeviceMessageSending;
	m_bWaypointSending = true;
	qDebug() << "航点数量" << ui.labelDeviceName->text() << count << _CurrentTime_;
	//先发送航点总数，消息响应成功后才能上传航点
	mavlink_message_t msg;
	mavlink_mission_count_t mission_count;
	mission_count.target_system = _DeviceSYS_ID_;
	mission_count.target_component = _DeviceCOMP_ID_;
	mission_count.count = count;
	int len = mavlink_msg_mission_count_encode(_DeviceSYS_ID_, _DeviceCOMP_ID_, &msg, &mission_count);
	QByteArray arrData = mavMessageToBuffer(msg);
	ui.progressBar->setMaximum(count);
	ui.progressBar->setValue(0);
	ui.progressBar->setVisible(true);
	m_currentWaypointData = data;
	ResendMessage* pMessageThread = new ResendMessage(tr("准备上传舞步"), ui.labelDeviceName->text(), _MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrData, MAVLINK_MSG_ID_MISSION_COUNT);
	connect(this, SIGNAL(sigCommandResult(QString, int, int)), pMessageThread, SLOT(onResult(QString, int, int)));
	connect(pMessageThread, SIGNAL(sigSendMessage(QByteArray)), this, SLOT(sendMessage(QByteArray)));
	connect(pMessageThread, SIGNAL(finished()), this, SLOT(onWaypointNext()));
	pMessageThread->start();
	return DeviceDataSucceed;
}

//无人机起飞
int DeviceControl::Fun_MAV_CMD_NAV_TAKEOFF_LOCAL(float Pitch, float Empty, float AscendRate, float Yaw, float X, float Y, float Z, bool wait, bool again)
{
	//参数1无效
	//参数2用了标记是否为重发的消息
	QByteArray arrData = mavCommandLongToBuffer(0, 0, AscendRate, Yaw, X, Y, Z, MAV_CMD_NAV_TAKEOFF_LOCAL);
	QByteArray againData = mavCommandLongToBuffer(0, _MavResendFlag_, AscendRate, Yaw, X, Y, Z, MAV_CMD_NAV_TAKEOFF_LOCAL);
	return MavSendCommandLongMessage(tr("起飞"), MAV_CMD_NAV_TAKEOFF_LOCAL, arrData, again ? arrData : "", wait);
}

//无人机降落
int DeviceControl::Fun_MAV_CMD_NAV_LAND_LOCAL(float Target, float Offset, float DescendRate, float Yaw, float X, float Y, float Z, bool wait, bool again)
{
	//参数1参数2无效
	QByteArray arrData = mavCommandLongToBuffer(0, 0, DescendRate, Yaw, X, Y, Z, MAV_CMD_NAV_LAND_LOCAL);
	return MavSendCommandLongMessage(tr("降落"), MAV_CMD_NAV_LAND_LOCAL, arrData, again ? arrData : "", wait);
}

//无人机急停
int DeviceControl::Fun_MAV_QUICK_STOP(bool wait, bool again)
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, _MavStopFlyMessageID_);
	return MavSendCommandLongMessage(tr("急停"), _MavStopFlyMessageID_, arrData, again ? arrData : "", wait, _MavLinkResendNum_, _MavQuickStopFlyTimeout_);
}

//无人机航点指令
int DeviceControl::Fun_MAV_CMD_NAV_WAYPOINT(float Hold, float AcceptRadius, float PassRadius, float Yaw, float Latitude, float Longitude, float Altitude, bool wait, bool again)
{
	QByteArray arrData = mavCommandLongToBuffer(Hold, AcceptRadius, PassRadius, Yaw, Latitude, Longitude, Altitude, MAV_CMD_NAV_WAYPOINT);
	return MavSendCommandLongMessage(tr("舞步"), MAV_CMD_NAV_WAYPOINT, arrData, again ? arrData : "", wait);
}

//无人机校准
int DeviceControl::Fun_MAV_CALIBRATION(float p1, float p2, float p3, float p4, float p5, float p6, float p7, bool wait, bool again)
{
	QByteArray arrData = mavCommandLongToBuffer(p1, p2, p3, p4, p5, p6, p7, MAV_CMD_PREFLIGHT_CALIBRATION);
	return MavSendCommandLongMessage(tr("校准"), MAV_CMD_PREFLIGHT_CALIBRATION, arrData, again ? arrData : "", wait);
}

//无人机列队
int DeviceControl::Fun_MAV_Defined_Queue()
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_USER_1);
	return MavSendCommandLongMessage(tr("列队"), MAV_CMD_USER_1, arrData, arrData, false);
}

//无人机回收
int DeviceControl::Fun_MAV_Defined_Regain()
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_USER_2);
	return MavSendCommandLongMessage(tr("回收"), MAV_CMD_USER_2, arrData, arrData, false);
}

//无人机灯光
int DeviceControl::Fun_MAV_LED_MODE()
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_WAYPOINT_USER_4);
	return MavSendCommandLongMessage(tr("灯光"), MAV_CMD_WAYPOINT_USER_4, arrData, arrData, false);
}

void DeviceControl::onUpdateBatteryStatus(float voltages, float battery, unsigned short electric)
{
	ui.labelBattery->setText(QString(tr("电量：%1 %")).arg(electric));
	m_pDebugDialog->onSetBatteryStatus(voltages, battery, electric);
}

//网络连接状态
void DeviceControl::hvcbConnectionStatus(const hv::SocketChannelPtr& channel)
{//子线程
	std::string peeraddr = channel->peeraddr();
	if (channel->isConnected()) {
		//连接成功
		channel->setHeartbeat(_DeviceHeartbeatInterval_, [&, this]() {
			//设置心跳
			if (!m_bHeartbeatEnable) return;
			QTime time = QTime::currentTime();
			mavlink_message_t message;
			mavlink_heartbeat_t heard;
			heard.custom_mode = time.minute() * 100 * 1000 + time.second() * 1000 + time.msec();
			heard.type = 17;
			heard.autopilot = 84;
			heard.base_mode = 151;
			heard.system_status = 218;
			if (mavlink_msg_heartbeat_encode(_DeviceSYS_ID_, _DeviceCOMP_ID_, &message, &heard) > 0) {
				QByteArray arrData = mavMessageToBuffer(message);
				m_pHvTcpClient->send(arrData.data(), arrData.length());
				//qDebug() << time<<"心跳消息" << arrData;
			}
			});
	}
	emit sigConnectStatus(ui.labelDeviceName->text(), peeraddr.c_str(), channel->isConnected());
}

//收到的数据
void DeviceControl::hvcbReceiveMessage(const hv::SocketChannelPtr& channel, hv::Buffer* buf)
{//子线程
	if (buf->size() <= 0) return;
	QByteArray arrData((char*)buf->data(), buf->size());
	if (arrData.isEmpty()) return;
	QString qstrName = ui.labelDeviceName->text();
	qDebug() << "收到消息" << qstrName << channel->peeraddr().c_str() << arrData.toHex().toUpper();
	QByteArray arrTemp = QString(_DeviceLogPrefix_).toLocal8Bit();
	if (arrData.contains(arrTemp)) {
		//日志数据
		QByteArray arrLog = arrData;
		while (arrLog.contains(arrTemp)) {
			int indexStart = arrLog.indexOf(arrTemp);
			int indexEnd = arrLog.indexOf(_DeviceLogEnd_) + 1;
			if (0 == indexEnd) indexEnd = arrLog.length();
			QByteArray temp = arrLog.mid(indexStart, indexEnd);
			emit sigLogMessage(temp);
			arrLog = arrLog.right(arrLog.length() - indexEnd);
		}
	}
	
	//解包
	mavlink_message_t msg;
	mavlink_status_t status;
	for (int i = 0; i < arrData.length(); i++) {
		uint8_t par = mavlink_parse_char(0, arrData[i], &msg, &status);
		if (!par) continue;
		qDebug() << "消息类型" << qstrName << msg.msgid;
		switch (msg.msgid)
		{
		case MAVLINK_MSG_ID_HEARTBEAT://心跳
			mavlink_heartbeat_t heart;
			mavlink_msg_heartbeat_decode(&msg, &heart);
			emit sigUpdateHeartbeat();
			break;
		case MAVLINK_MSG_ID_MISSION_COUNT:  //航点起始应答
			mavlink_mission_count_t missionCount;
			mavlink_msg_mission_count_decode(&msg, &missionCount);
			emit sigMessageByte(mavMessageToBuffer(msg), true);
			//0成功,成功后发送航点数据
			emit sigCommandResult(ui.labelDeviceName->text(), missionCount.count>0?0:-1, MAVLINK_MSG_ID_MISSION_COUNT);
			break;
		case MAVLINK_MSG_ID_MISSION_ACK:    //航点应答
			mavlink_mission_ack_t ack;
			mavlink_msg_mission_ack_decode(&msg, &ack);
			emit sigMessageByte(mavMessageToBuffer(msg), true);
			emit sigCommandResult(ui.labelDeviceName->text(), ack.type, MAVLINK_MSG_ID_MISSION_ACK);
			break;
		case MAVLINK_MSG_ID_COMMAND_ACK:	//命令应答
		{
			mavlink_command_ack_t ack;
			mavlink_msg_command_ack_decode(&msg, &ack);
			emit sigMessageByte(mavMessageToBuffer(msg), true);
			emit sigCommandResult(ui.labelDeviceName->text(), ack.result, ack.command);
			break;		
		}
		case MAVLINK_MSG_ID_BATTERY_STATUS:  //电池信息
		{
			mavlink_battery_status_t battery;
			mavlink_msg_battery_status_decode(&msg, &battery);
			//电压范围9.6-12.6
			uint16_t v = battery.voltages[0];
			int16_t b = battery.current_battery;
			//计算剩余电量
			uint16_t n = (float(v - 9600) / (12600 - 9600)) * 100;
			emit sigBatteryStatus((float)v / 1000, (float)b / 1000, n);
			m_deviceStatus.battery = n;
			break;
		}
		case MAVLINK_MSG_ID_ATTITUDE:	//姿态角
			mavlink_attitude_t attitude;
			mavlink_msg_attitude_decode(&msg, &attitude);
			m_deviceStatus.roll = attitude.roll;
			m_deviceStatus.pitch = attitude.pitch;
			m_deviceStatus.yaw = attitude.yaw;
			emit sigAttitude(attitude.time_boot_ms, attitude.roll, attitude.pitch, attitude.yaw);
			break;
		case MAVLINK_MSG_ID_LOCAL_POSITION_NED: //位置信息
		{
			mavlink_local_position_ned_t t;
			mavlink_msg_local_position_ned_decode(&msg, &t);
			m_deviceStatus.x = QString::number(t.x, 'f', 3).toFloat() * 10;
			m_deviceStatus.y = QString::number(t.y, 'f', 3).toFloat() * 10;
			m_deviceStatus.z = QString::number(t.z, 'f', 3).toFloat() * 10;
			emit sigLocalPosition(t.time_boot_ms, QString::number(t.x, 'f', 3).toFloat(), QString::number(t.y, 'f', 3).toFloat(), QString::number(t.z, 'f', 3).toFloat());
			break;
		}
		case MAVLINK_MSG_ID_HIGHRES_IMU:	//IMU数据
		{
			mavlink_highres_imu_t t;
			mavlink_msg_highres_imu_decode(&msg, &t);
			QList<float> list;
			list << t.xacc << t.yacc << t.zacc << t.xgyro << t.ygyro << t.zgyro << t.xmag << t.ymag << t.zmag;
			emit sigHighresImu(t.time_usec, list);
			break;
		}
		default:
			break;
		}
	}
}

//已经发送的数据
void DeviceControl::hvcbWriteComplete(const hv::SocketChannelPtr& channel, hv::Buffer* buf)
{//子线程
	if (buf->size() <= 0) return;
	QByteArray arrBuf((char*)buf->data(), buf->size());
	if (arrBuf.isEmpty()) return;
	emit sigMessageByte(arrBuf, false);
	qDebug() << "已发送消息" << ui.labelDeviceName->text() << channel->peeraddr().c_str() << arrBuf.toHex().toUpper();
}

QByteArray DeviceControl::mavMessageToBuffer(mavlink_message_t mesage)
{
	//需要加锁
	QMutexLocker locker(&m_mutexMavMessageToBurrer);
	uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = { 0 };
	int len = mavlink_msg_to_send_buffer(buffer, &mesage);
	QByteArray arrData = QByteArray((char*)buffer, len);
	return arrData;
}

QByteArray DeviceControl::mavCommandLongToBuffer(float param1, float param2, float param3, float param4, float param5, float param6, float param7, int command, int confirmation /*= 1*/)
{
	mavlink_message_t message;
	mavlink_command_long_t cmd;
	cmd.param1 = param1;
	cmd.param2 = param2;
	cmd.param3 = param3;
	cmd.param4 = param4;
	cmd.param5 = param5;
	cmd.param6 = param6;
	cmd.param7 = param7;
	cmd.command = command;
	cmd.target_system = _DeviceSYS_ID_;
	cmd.target_component = _DeviceCOMP_ID_;
	cmd.confirmation = confirmation;
	if(0 >= mavlink_msg_command_long_encode(_DeviceSYS_ID_, _DeviceCOMP_ID_, &message, &cmd)) return "";
	return mavMessageToBuffer(message);
}

int DeviceControl::MavSendCommandLongMessage(QString name, int commandID, QByteArray arrData, QByteArray arrAgainData, bool bWait
	, unsigned int againNum, unsigned int againInterval)
{
	if (arrData.isEmpty()) return DeviceDataError;
	if (!isConnectDevice()) return DeviceUnConnect;
	//通过MessageThread控制重发数据及处理消息响应
	ResendMessage* pMessageThread = new ResendMessage(name, ui.labelDeviceName->text(), arrAgainData.isEmpty() ? 0 : againNum, againInterval, arrData, arrAgainData, commandID);
	//接收设备回复的消息响应
	connect(this, SIGNAL(sigCommandResult(QString, int, int)), pMessageThread, SLOT(onResult(QString, int, int)));
	connect(pMessageThread, SIGNAL(sigSendMessage(QByteArray)), this, SLOT(sendMessage(QByteArray)));
	pMessageThread->start();

	if (false == bWait) {
		//当使用重发消息但不等待返回结果时，MessageThread指针无法销毁，设置自动销毁，使用完后自动注销
		//pMessageThread->setAutoDelete(true);
		//直接返回结果，线程超时或收到响应后自动销毁
		connect(pMessageThread, &QThread::finished, this, &DeviceControl::onMessageThreadFinished);
		return DeviceDataSucceed;
	}
	//阻塞等待结果
	while (!pMessageThread->isFinished()) {
		QApplication::processEvents();
	}
	//线程结果后查询返回结果
	int res = pMessageThread->getResult();
	pMessageThread->deleteLater();
	pMessageThread = nullptr;
	return res;
}

QByteArray DeviceControl::getWaypointData(float param1, float param2, float param3, float param4, int32_t x, int32_t y, float z, uint16_t seq, unsigned int again)
{
	mavlink_message_t msg;
	mavlink_mission_item_int_t mission;
	mission.param1 = param1;
	mission.param2 = param2;
	mission.param3 = param3;
	mission.param4 = param4;
	mission.x = x;
	mission.y = y;
	mission.z = z;
	mission.seq = seq;
	mission.command = MAV_CMD_NAV_WAYPOINT;
	mission.target_system = _DeviceSYS_ID_;
	mission.target_component = _DeviceCOMP_ID_;
	mission.current = again;
	int len = mavlink_msg_mission_item_int_encode(_DeviceSYS_ID_, _DeviceCOMP_ID_, &msg, &mission);
	return mavMessageToBuffer(msg);
}

void DeviceControl::DeviceMavWaypointSend(QVector<NavWayPointData> data)
{
	int count = data.count();
	if (false == m_bWaypointSending || count <= 0) {
		m_bWaypointSending = false;
		ui.progressBar->setVisible(false);
		emit sigWaypointProcess(ui.labelDeviceName->text(), 0, count, DeviceMessageSending, true, tr("上传舞步中"));
		return;
	}
	qDebug() << "0号航点" << ui.labelDeviceName->text() << _CurrentTime_;
	//先发送0号全零航点，验证是否可以上传航点
	QByteArray arrData = getWaypointData(0, 0, 0, 0, 0, 0, 0, 0, 0);
	QByteArray arrAgainData = getWaypointData(0, 0, 0, 0, 0, 0, 0, 0, 1);
	ResendMessage* pMessageThread = new ResendMessage(tr("上传预制舞步"), ui.labelDeviceName->text(), _MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrAgainData, MAVLINK_MSG_ID_MISSION_ACK);
	//接收设备回复的消息响应
	connect(this, SIGNAL(sigCommandResult(QString, int, int)), pMessageThread, SLOT(onResult(QString, int, int)));
	connect(pMessageThread, SIGNAL(sigSendMessage(QByteArray)), this, SLOT(sendMessage(QByteArray)), Qt::DirectConnection);
	pMessageThread->start();
	//阻塞等待结果
	while (!pMessageThread->isFinished()) {
		QApplication::processEvents();
	}
	//线程结果后查询返回结果
	int res = pMessageThread->getResult();
	pMessageThread->deleteLater();
	if (DeviceDataSucceed != res) {
		//上传航点失败或超时，整个过程结束
		m_bWaypointSending = false;
		ui.progressBar->setVisible(false);
		emit sigWaypointProcess(ui.labelDeviceName->text(), 0, count, res, true, tr("0号舞步"));
		return;
	}
	QString qstrTemp = ui.labelDeviceName->text();
	qDebug() << "循环发送航点" << ui.labelDeviceName->text() << _CurrentTime_;
	//开始上传航点,循环发送航点数据
	for (int i = 0; i < count; i++) {
		qDebug() << "舞步序号" << ui.labelDeviceName->text() << i + 1<< _CurrentTime_;
		NavWayPointData temp = data.at(i);
		//与设备通讯协议中规定X与Y值需要*1000
		QByteArray arrData = getWaypointData(temp.param1, temp.param2, temp.param3, temp.param4, temp.x * 1000, temp.y * 1000, temp.z, i + 1, 0);
		QByteArray arrAgainData = getWaypointData(temp.param1, temp.param2, temp.param3, temp.param4, temp.x * 1000, temp.y * 1000, temp.z, i + 1, 1);
		ResendMessage* pWaypointThread = new ResendMessage(tr("上传舞步"), ui.labelDeviceName->text(), _MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrAgainData, MAVLINK_MSG_ID_MISSION_ACK);
		connect(this, SIGNAL(sigCommandResult(QString, int, int)), pWaypointThread, SLOT(onResult(QString, int, int)));
		//阻塞发送
		connect(pWaypointThread, SIGNAL(sigSendMessage(QByteArray)), this, SLOT(sendMessage(QByteArray)), Qt::DirectConnection);
		pWaypointThread->start();
		while (!pWaypointThread->isFinished()) {
			QApplication::processEvents();
		}
		int res = pWaypointThread->getResult();
		pWaypointThread->deleteLater();
		if (DeviceDataSucceed != res) {
			//上传航点失败或超时，整个过程结束
			m_bWaypointSending = false;
			ui.progressBar->setVisible(false);
			emit sigWaypointProcess(ui.labelDeviceName->text(), i + 1, count, res, true, tr("上传第")+QString::number(i+1)+tr("条舞步"));
			return;
		}
		emit sigWaypointProcess(ui.labelDeviceName->text(), i + 1, count, res, false, tr(""));
	}

	//航点下发完成后需要发送结束信息
	DeviceMavWaypointEnd(count);
}

void DeviceControl::DeviceMavWaypointEnd(unsigned int count)
{
	qDebug() << "结束舞步" << ui.labelDeviceName->text() << _CurrentTime_;
	mavlink_message_t msg;
	mavlink_mission_ack_t ack;
	ack.target_system = _DeviceSYS_ID_;
	ack.target_component = _DeviceCOMP_ID_;
	ack.type = MAV_MISSION_ACCEPTED;
	mavlink_msg_mission_ack_encode(_DeviceSYS_ID_, _DeviceCOMP_ID_, &msg, &ack);
	QByteArray arrData = mavMessageToBuffer(msg);
	QByteArray arrAgainData = arrData;
	ResendMessage* pMessageThread = new ResendMessage(tr("结束舞步上传"), ui.labelDeviceName->text(), _MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrAgainData, MAVLINK_MSG_ID_MISSION_ACK);
	//接收设备回复的消息响应
	connect(this, SIGNAL(sigCommandResult(QString, int, int)), pMessageThread, SLOT(onResult(QString, int, int)));
	connect(pMessageThread, SIGNAL(sigSendMessage(QByteArray)), this, SLOT(sendMessage(QByteArray)), Qt::DirectConnection);
	pMessageThread->start();
	//阻塞等待结果
	while (!pMessageThread->isFinished()) {
		QApplication::processEvents();
	}
	//线程结果后查询返回结果
	int res = pMessageThread->getResult();
	pMessageThread->deleteLater();
	//整个上传航点过程结束
	m_bWaypointSending = false;
	emit sigWaypointProcess(ui.labelDeviceName->text(), count, count, res, true, tr("结束上传舞步"));
	ui.progressBar->setVisible(false);
}

void DeviceControl::onUpdateHeartBeat()
{
	unsigned int nTime = QDateTime::currentDateTime().toTime_t();
	m_timerHeartbeat.setProperty("time", nTime);
	ui.labelStatus->setVisible(true);
	QString qstrImagePath = ":/res/images/uavgreen.png";
	QString qstrCurrent = ui.labelStatus->property("iamgepath").toString();
	if (qstrCurrent == qstrImagePath) {
		qstrImagePath = ":/res/images/uavyellow.png";
	}
	else {
		qstrImagePath = ":/res/images/uavgreen.png";
	}
	QPixmap pixmap(qstrImagePath);
	ui.labelStatus->setPixmap(pixmap.scaled(ui.labelStatus->size()));
	ui.labelStatus->setProperty("iamgepath", qstrImagePath);
}

void DeviceControl::onUpdateConnectStatus(QString name, QString ip, bool connect)
{
	if (connect) {
		m_timerHeartbeat.setProperty("time", QDateTime::currentDateTime().toTime_t());
		m_timerHeartbeat.start(_DeviceHeartbeatInterval_);
		QPixmap pixmap(":/res/images/uavgreen.png");
		ui.labelStatus->setPixmap(pixmap.scaled(ui.labelStatus->size()));
	}
	else {
		m_timerHeartbeat.stop();
		QPixmap pixmap(":/res/images/uavred.png");
		ui.labelStatus->setPixmap(pixmap.scaled(ui.labelStatus->size()));
		
	}
}

void DeviceControl::onWaypointProcess(QString name, unsigned int index, unsigned int count, int res, bool finish, QString text)
{
	ui.progressBar->setValue(index);
}

void DeviceControl::onWaypointNext()
{
	ResendMessage* pMessage = dynamic_cast<ResendMessage*>(sender());
	if (!pMessage) return;
	int res = pMessage->getResult();
	if (DeviceDataSucceed != res) {
		m_bWaypointSending = false;
		ui.progressBar->setVisible(false);
		emit sigWaypointProcess(ui.labelDeviceName->text(), 0, m_currentWaypointData.count(), res, true, tr("请求上传舞步"));
		return;
	}
	delete pMessage;
	//成功后开始上传航点
	DeviceMavWaypointSend(m_currentWaypointData);
}

void DeviceControl::onMessageThreadFinished()
{
	ResendMessage* pMessage = dynamic_cast<ResendMessage*>(sender());
	if (!pMessage) return;
	emit sigConrolFinished(pMessage->getCommandName(), pMessage->getResult(), "");
	delete pMessage;
}


