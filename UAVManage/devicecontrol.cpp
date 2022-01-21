#include "devicecontrol.h"
#include <QDebug>
#include <QTime>
#include <QDateTime>

#define _CurrentTime_  QTime::currentTime().toString("hh:mm:ss.zzz")
DeviceControl::DeviceControl(QString name, float x, float y, QString ip, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_pHvTcpClient = nullptr;
	m_pHvTcpClient = new hv::TcpClient;
	m_bHeartbeatEnable = true;
	m_bWaypointSending = false;
	setName(name);
	setIp(ip);
	setX(x);
	setY(y);
}

DeviceControl::~DeviceControl()
{
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

float DeviceControl::getX()
{
	return m_fStartX;
}

void DeviceControl::setX(float x)
{
	m_fStartX = x;
}

float DeviceControl::getY()
{
	return m_fStartY;
}

void DeviceControl::setY(float y)
{
	m_fStartY = y;
}

void DeviceControl::setStartLocation(float x, float y)
{
	setX(x);
	setY(y);
}

QList<float> DeviceControl::getStartLocation()
{
	QList<float> list;
	list << getX() << getY();
	return list;
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
	return MavSendCommandLongMessage(MAV_CMD_DO_SET_MODE, arrData, again ? arrData : "", false);
}

int DeviceControl::DeviceMavWaypointStart(QVector<NavWayPointData> data)
{
	qDebug() << "----准备发送航点" << ui.labelDeviceName->text();
	if (!isConnectDevice()) return DeviceUnConnect;
	int count = data.count();
	if (count <= 0) return DeviceDataError;

	//标记航点下发是否进行中
	if (m_bWaypointSending) return DeviceMessageSending;
	m_bWaypointSending = true;
	qDebug() << "----航点数量" << ui.labelDeviceName->text() << count;
	//先发送航点总数，消息响应成功后才能下发航点
	mavlink_message_t msg;
	mavlink_mission_count_t mission_count;
	mission_count.target_system = _DeviceSYS_ID_;
	mission_count.target_component = _DeviceCOMP_ID_;
	mission_count.count = count;
	int len = mavlink_msg_mission_count_encode(_DeviceSYS_ID_, _DeviceCOMP_ID_, &msg, &mission_count);
	QByteArray arrData = mavMessageToBuffer(msg);

	ResendMessage* pMessageThread = new ResendMessage(_MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrData, MAVLINK_MSG_ID_MISSION_COUNT);
	connect(this, SIGNAL(sigCommandResult(QString, int, int)), pMessageThread, SLOT(onResult(QString, int, int)));
	connect(pMessageThread, SIGNAL(sigSendMessage(QByteArray)), this, SLOT(sendMessage(QByteArray)));
	pMessageThread->start();
	//线程完成，超时或者收到消息响应
	connect(pMessageThread, &QThread::finished, [=]() {
		//开始下发航点
		int res = pMessageThread->getResult();
		pMessageThread->deleteLater();
		qDebug() << "----下发航点结果" << ui.labelDeviceName->text() << res;
		if (DeviceDataSucceed != res) {
			m_bWaypointSending = false;
			emit sigWaypointProcess(ui.labelDeviceName->text(), 0, count, res, true, tr("请求下发航点"));
			return;
		}
		//成功后开始下发航点
		DeviceMavWaypointSend(data);		
		});
	return DeviceDataSucceed;
}

//无人机起飞
int DeviceControl::Fun_MAV_CMD_NAV_TAKEOFF_LOCAL(float Pitch, float Empty, float AscendRate, float Yaw, float X, float Y, float Z, bool wait, bool again)
{
	//参数1无效
	//参数2用了标记是否为重发的消息
	QByteArray arrData = mavCommandLongToBuffer(0, 0, AscendRate, Yaw, X, Y, Z, MAV_CMD_NAV_TAKEOFF_LOCAL);
	QByteArray againData = mavCommandLongToBuffer(0, _MavResendFlag_, AscendRate, Yaw, X, Y, Z, MAV_CMD_NAV_TAKEOFF_LOCAL);
	return MavSendCommandLongMessage(MAV_CMD_NAV_TAKEOFF_LOCAL, arrData, again ? arrData : "", false);
}

//无人机降落
int DeviceControl::Fun_MAV_CMD_NAV_LAND_LOCAL(float Target, float Offset, float DescendRate, float Yaw, float X, float Y, float Z, bool wait, bool again)
{
	//参数1参数2无效
	QByteArray arrData = mavCommandLongToBuffer(0, 0, DescendRate, Yaw, X, Y, Z, MAV_CMD_NAV_LAND_LOCAL);
	return MavSendCommandLongMessage(MAV_CMD_NAV_LAND_LOCAL, arrData, again ? arrData : "", false);
}

//无人机急停
int DeviceControl::Fun_MAV_QUICK_STOP(bool wait, bool again)
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, _MavStopFlyMessageID_);
	return MavSendCommandLongMessage(_MavStopFlyMessageID_, arrData, again ? arrData : "", false, _MavLinkResendNum_, _MavQuickStopFlyTimeout_);
}

//无人机航点指令
int DeviceControl::Fun_MAV_CMD_NAV_WAYPOINT(float Hold, float AcceptRadius, float PassRadius, float Yaw, float Latitude, float Longitude, float Altitude, bool wait, bool again)
{
	QByteArray arrData = mavCommandLongToBuffer(Hold, AcceptRadius, PassRadius, Yaw, Latitude, Longitude, Altitude, MAV_CMD_NAV_WAYPOINT);
	return MavSendCommandLongMessage(MAV_CMD_NAV_WAYPOINT, arrData, again ? arrData : "", false);
}

//无人机校准
int DeviceControl::Fun_MAV_CALIBRATION(float p1, float p2, float p3, float p4, float p5, float p6, float p7, bool wait, bool again)
{
	QByteArray arrData = mavCommandLongToBuffer(p1, p2, p3, p4, p5, p6, p7, MAV_CMD_PREFLIGHT_CALIBRATION);
	return MavSendCommandLongMessage(MAV_CMD_PREFLIGHT_CALIBRATION, arrData, again ? arrData : "", false);
}

//无人机列队
int DeviceControl::Fun_MAV_Defined_Queue()
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_USER_1);
	return MavSendCommandLongMessage(MAV_CMD_USER_1, arrData, arrData, false);
}

//无人机回收
int DeviceControl::Fun_MAV_Defined_Regain()
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_USER_2);
	return MavSendCommandLongMessage(MAV_CMD_USER_2, arrData, arrData, false);
}

//无人机灯光
int DeviceControl::Fun_MAV_LED_MODE()
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_WAYPOINT_USER_4);
	return MavSendCommandLongMessage(MAV_CMD_WAYPOINT_USER_4, arrData, arrData, false);
}

//网络连接状态
void DeviceControl::hvcbConnectionStatus(const hv::SocketChannelPtr& channel)
{//子线程
	std::string peeraddr = channel->peeraddr();
	if (channel->isConnected()) {
		//连接成功
		channel->setHeartbeat(_DeviceHeartbeatInterval_, [&, this]() {
			//设置心跳
			return;
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
			}
			});
		ui.labelConnect->setText(tr("已连接"));
	}
	else {
		//连接断开
		ui.labelConnect->setText(tr("已断开"));
	}
	emit sigConnectStatus(ui.labelDeviceName->text(), peeraddr.c_str(), channel->isConnected());
}

//收到的数据
void DeviceControl::hvcbReceiveMessage(const hv::SocketChannelPtr& channel, hv::Buffer* buf)
{//子线程
	if (buf->size() <= 0) return;
	QByteArray arrData((char*)buf->data(), buf->size());
	if (arrData.isEmpty()) return;
	qDebug() << "----收到消息" << _CurrentTime_ << channel->peeraddr().c_str() << arrData.toHex().toUpper();
	//解包
	mavlink_message_t msg;
	mavlink_status_t status;
	for (int i = 0; i < arrData.length(); i++) {
		uint8_t par = mavlink_parse_char(0, arrData[i], &msg, &status);
		if (!par) continue;
		switch (msg.msgid)
		{
		case MAVLINK_MSG_ID_HEARTBEAT://心跳
			mavlink_heartbeat_t heart;
			mavlink_msg_heartbeat_decode(&msg, &heart);
			break;
		case MAVLINK_MSG_ID_MISSION_COUNT:  //航点起始应答
			mavlink_mission_count_t missionCount;
			mavlink_msg_mission_count_decode(&msg, &missionCount);
			//0成功,成功后发送航点数据
			emit sigCommandResult(ui.labelDeviceName->text(), missionCount.count>0?0:-1, MAVLINK_MSG_ID_MISSION_COUNT);
			break;
		case MAVLINK_MSG_ID_MISSION_ACK:    //航点应答
			mavlink_mission_ack_t ack;
			mavlink_msg_mission_ack_decode(&msg, &ack);
			emit sigCommandResult(ui.labelDeviceName->text(), ack.type, MAVLINK_MSG_ID_MISSION_ACK);
			if (MAV_MISSION_ACCEPTED == ack.type); //成功下一条
			break;
		case MAVLINK_MSG_ID_COMMAND_ACK:	//命令应答
		{
			mavlink_command_ack_t ack;
			mavlink_msg_command_ack_decode(&msg, &ack);
			emit sigCommandResult(ui.labelDeviceName->text(), ack.result, ack.command);
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
	qDebug() << "----已发送消息" << _CurrentTime_<< channel->peeraddr().c_str() << arrBuf.toHex().toUpper();
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

int DeviceControl::MavSendCommandLongMessage(int commandID, QByteArray arrData, QByteArray arrAgainData, bool bWait
	, unsigned int againNum, unsigned int againInterval)
{
	if (arrData.isEmpty()) return DeviceDataError;
	if (!isConnectDevice()) return DeviceUnConnect;
	//通过MessageThread控制重发数据及处理消息响应
	ResendMessage* pMessageThread = new ResendMessage(arrAgainData.isEmpty() ? 0 : againNum, againInterval, arrData, arrAgainData, commandID);
	//接收设备回复的消息响应
	connect(this, SIGNAL(sigCommandResult(QString, int, int)), pMessageThread, SLOT(onResult(QString, int, int)));
	connect(pMessageThread, SIGNAL(sigSendMessage(QByteArray)), this, SLOT(sendMessage(QByteArray)));
	pMessageThread->start();

	if (false == bWait) {
		//当使用重发消息但不等待返回结果时，MessageThread指针无法销毁，设置自动销毁，使用完后自动注销
		pMessageThread->setAutoDelete(true);
		//直接返回结果，线程超时或收到响应后自动销毁
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
		emit sigWaypointProcess(ui.labelDeviceName->text(), 0, count, DeviceMessageSending, true, tr("下发航点中"));
		return;
	}
	//先发送0号全零航点，验证是否可以下发航点
	QByteArray arrData = getWaypointData(0, 0, 0, 0, 0, 0, 0, 0, 0);
	QByteArray arrAgainData = getWaypointData(0, 0, 0, 0, 0, 0, 0, 0, 1);
	ResendMessage* pMessageThread = new ResendMessage(_MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrAgainData, MAVLINK_MSG_ID_MISSION_ACK);
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
		//下发航点失败或超时，整个过程结束
		m_bWaypointSending = false;
		emit sigWaypointProcess(ui.labelDeviceName->text(), 0, count, res, true, tr("0号航点"));
		return;
	}
	//开始下发航点,循环发送航点数据
	for (int i = 0; i < count; i++) {
		NavWayPointData temp = data.at(i);
		QByteArray arrData = getWaypointData(temp.param1, temp.param2, temp.param3, temp.param4, temp.x, temp.y, temp.z, i + 1, 0);
		QByteArray arrAgainData = getWaypointData(temp.param1, temp.param2, temp.param3, temp.param4, temp.x, temp.y, temp.z, i + 1, 1);
		ResendMessage* pWaypointThread = new ResendMessage(_MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrAgainData, MAVLINK_MSG_ID_MISSION_ACK);
		connect(this, SIGNAL(sigCommandResult(QString, int, int)), pWaypointThread, SLOT(onResult(QString, int, int)));
		connect(pWaypointThread, SIGNAL(sigSendMessage(QByteArray)), this, SLOT(sendMessage(QByteArray)), Qt::DirectConnection);
		pWaypointThread->start();
		while (!pWaypointThread->isFinished()) {
			QApplication::processEvents();
		}
		int res = pWaypointThread->getResult();
		pWaypointThread->deleteLater();
		if (DeviceDataSucceed != res) {
			//下发航点失败或超时，整个过程结束
			m_bWaypointSending = false;
			emit sigWaypointProcess(ui.labelDeviceName->text(), i + 1, count, res, true, tr("下发航点过程"));
			return;
		}
		emit sigWaypointProcess(ui.labelDeviceName->text(), i + 1, count, res, false, tr(""));
	}

	//航点下发完成后需要发送结束信息
	DeviceMavWaypointEnd(count);
}

void DeviceControl::DeviceMavWaypointEnd(unsigned int count)
{
	mavlink_message_t msg;
	mavlink_mission_ack_t ack;
	ack.target_system = _DeviceSYS_ID_;
	ack.target_component = _DeviceCOMP_ID_;
	ack.type = MAV_MISSION_ACCEPTED;
	mavlink_msg_mission_ack_encode(_DeviceSYS_ID_, _DeviceCOMP_ID_, &msg, &ack);
	QByteArray arrData = mavMessageToBuffer(msg);
	QByteArray arrAgainData = arrData;
	ResendMessage* pMessageThread = new ResendMessage(_MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrAgainData, MAVLINK_MSG_ID_MISSION_ACK);
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
	//整个下发航点过程结束
	m_bWaypointSending = false;
	emit sigWaypointProcess(ui.labelDeviceName->text(), count, count, res, true, tr("下发航点完成"));
}
