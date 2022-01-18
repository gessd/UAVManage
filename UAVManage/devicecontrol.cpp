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
	emit sigRenewTcpClient();
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
	if (nullptr == m_pHvTcpClient) return false;
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
	return MavSendCommandLongMessage(MAV_CMD_DO_SET_MODE, arrData, arrData, wait, again, _MavLinkResendNum_, _NkCommandResendInterval_, _NkCommandResendInterval_);
}

//无人机起飞
int DeviceControl::Fun_MAV_CMD_NAV_TAKEOFF_LOCAL(float Pitch, float Empty, float AscendRate, float Yaw, float X, float Y, float Z, bool wait, bool again)
{
	//参数1无效
	//参数2用了标记是否为重发的消息
	QByteArray arrData = mavCommandLongToBuffer(0, 0, AscendRate, Yaw, X, Y, Z, MAV_CMD_NAV_TAKEOFF_LOCAL);
	QByteArray againData = mavCommandLongToBuffer(0, _MavResendFlag_, AscendRate, Yaw, X, Y, Z, MAV_CMD_NAV_TAKEOFF_LOCAL);
	return MavSendCommandLongMessage(MAV_CMD_NAV_TAKEOFF_LOCAL, arrData, againData, wait, again, _MavLinkResendNum_, _NkCommandResendInterval_, _NkCommandResendInterval_);
}

//无人机降落
int DeviceControl::Fun_MAV_CMD_NAV_LAND_LOCAL(float Target, float Offset, float DescendRate, float Yaw, float X, float Y, float Z, bool wait, bool again)
{
	//参数1参数2无效
	QByteArray arrData = mavCommandLongToBuffer(0, 0, DescendRate, Yaw, X, Y, Z, MAV_CMD_NAV_LAND_LOCAL);
	return MavSendCommandLongMessage(MAV_CMD_NAV_LAND_LOCAL, arrData, arrData, wait, again, _MavLinkResendNum_, _NkCommandResendInterval_, _NkCommandResendInterval_);
}

//无人机急停
int DeviceControl::Fun_MAV_QUICK_STOP(bool wait, bool again)
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, _MavStopFlyMessageID_);
	return MavSendCommandLongMessage(_MavStopFlyMessageID_, arrData, arrData, wait, again, _MavLinkResendNum_, _MavQuickStopFlyTimeout_, _NkCommandResendInterval_);
}

//无人机航点指令
int DeviceControl::Fun_MAV_CMD_NAV_WAYPOINT(float Hold, float AcceptRadius, float PassRadius, float Yaw, float Latitude, float Longitude, float Altitude, bool wait, bool again)
{
	QByteArray arrData = mavCommandLongToBuffer(Hold, AcceptRadius, PassRadius, Yaw, Latitude, Longitude, Altitude, MAV_CMD_NAV_WAYPOINT);
	return MavSendCommandLongMessage(MAV_CMD_NAV_WAYPOINT, arrData, arrData, wait, again, _MavLinkResendNum_, _NkCommandResendInterval_, _NkCommandResendInterval_);
}

//无人机校准
int DeviceControl::Fun_MAV_CALIBRATION(float p1, float p2, float p3, float p4, float p5, float p6, float p7, bool wait, bool again)
{
	QByteArray arrData = mavCommandLongToBuffer(p1, p2, p3, p4, p5, p6, p7, MAV_CMD_PREFLIGHT_CALIBRATION);
	return MavSendCommandLongMessage(MAV_CMD_PREFLIGHT_CALIBRATION, arrData, arrData, wait, again, _MavLinkResendNum_, _NkCommandResendInterval_, _NkCommandResendInterval_);
}

//无人机列队
int DeviceControl::Fun_MAV_Defined_Queue()
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_USER_1);
	return MavSendCommandLongMessage(MAV_CMD_USER_1, arrData, arrData, true, true, _MavLinkResendNum_, _NkCommandResendInterval_, _NkCommandResendInterval_);
}

//无人机回收
int DeviceControl::Fun_MAV_Defined_Regain()
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_USER_2);
	return MavSendCommandLongMessage(MAV_CMD_USER_2, arrData, arrData, true, true, _MavLinkResendNum_, _NkCommandResendInterval_, _NkCommandResendInterval_);
}

//无人机灯光
int DeviceControl::Fun_MAV_LED_MODE()
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_WAYPOINT_USER_4);
	return MavSendCommandLongMessage(MAV_CMD_WAYPOINT_USER_4, arrData, arrData, true, true, _MavLinkResendNum_, _NkCommandResendInterval_, _NkCommandResendInterval_);
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
	, bool bAgainsend, unsigned int againNum, unsigned int againInterval, unsigned int nTimeout)
{
	if (arrData.isEmpty()) return DeviceDataError;
	if (bAgainsend && arrAgainData.isEmpty()) return DeviceDataError;
	if (!isConnectDevice()) return DeviceUnConnect;
	static bool bWaiting = true;
	bWaiting = true;
	int res = DeviceWaiting;
	//新此信号及接受者都为全局指针，顾此信号会出现重复绑定，通过静态bConnect控制只绑定一次信号槽
	static bool bConnect = false;
	if (false == bConnect) {
		//等待指令返回结果
		connect(this, &DeviceControl::sigCommandResult, [&, this](QString name, int result, int commandid) {
			if (false == bWaiting) return;
			if (commandID != commandid) return;
			res = result;
			});
		bConnect = true;
	}
	ResendMessage* pMessageThread = nullptr;
	if (bAgainsend) {
		//通过MessageThread控制重发数据及处理消息响应
		pMessageThread = new ResendMessage(m_pHvTcpClient, againNum, againInterval, arrData, arrAgainData, commandID);
		connect(this, SIGNAL(sigCommandResult(QString, int, int)), pMessageThread, SLOT(onResult(QString, int, int)));
		//连接主动断开时TCPclient指针会释放,需求停止线程,否则造成崩溃
		connect(this, SIGNAL(sigRenewTcpClient()), pMessageThread, SLOT(stopThread()));
		pMessageThread->start();
	} else
		m_pHvTcpClient->send(arrData.data(), arrData.length());

	if (false == bWait) {
		//当使用重发消息但不等待返回结果时，MessageThread指针无法销毁，设置自动销毁，使用完后自动注销
		if (pMessageThread) pMessageThread->setAutoDelete(true);
		return DeviceDataSucceed;
	}
	if (pMessageThread) {
		//阻塞等待结果
		while (!pMessageThread->isFinished()) {
			QApplication::processEvents();
		}
		bWaiting = false;
		int res = pMessageThread->getResult();
		pMessageThread->deleteLater();
		return res;
	}
	else {
		//当需要等待返回结果但不使用重发时，此处以定时器判断超时时间，直到消息返回结果
		QTime timeStart;
		timeStart.start();
		while (DeviceWaiting == res) {
			QApplication::processEvents();
			if (timeStart.elapsed() > nTimeout) {
				res = DeviceMessageToimeout;
				break;
			}
		}
		bWaiting = false;
		return res;
	}
	return DeviceWaiting;
}
