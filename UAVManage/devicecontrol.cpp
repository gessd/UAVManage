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
	static int chan = 0;
	//不同连接使用不同通道，防止数据混乱
	m_nMavChan = chan++;
	qDebug() << name <<"MAV通讯通道" << m_nMavChan;
	m_pHvTcpClient = new hv::TcpClient;
	m_bHeartbeatEnable = true;
	m_bWaypointSending = false;
	m_nCurrentWaypontIndex = -1;
	m_nCurrentMusicTime = 0;
	m_bUploadFinished = false;
	m_bPrepareTakeoff = false;
	m_bTimeSync = false;
	ui.stackedWidgetStatus->setCurrentIndex(0);
	QPixmap pixmap(":/res/images/uavred.png");
	ui.labelStatus->setPixmap(pixmap.scaled(ui.labelStatus->size()));
	//发送消息为了转移到主线程中处理，需要对界面进行处理
	connect(this, &DeviceControl::sigBatteryStatus, this, &DeviceControl::onUpdateBatteryStatus);
	connect(this, &DeviceControl::sigConnectStatus, this, &DeviceControl::onUpdateConnectStatus);
	connect(this, &DeviceControl::sigUpdateHeartbeat, this, &DeviceControl::onUpdateHeartBeat);
	connect(this, &DeviceControl::sigLocalPosition, this, &DeviceControl::onUpdateLocation);
	
	qRegisterMetaType<QList<float>>("QList<float>");
	m_pDebugDialog = new DeviceDebug(ip, name, this);
	connect(this, &DeviceControl::sigConnectStatus, m_pDebugDialog, &DeviceDebug::onConnectStatus);
	connect(this, &DeviceControl::sigMessageByte, m_pDebugDialog, &DeviceDebug::onDeviceMessage);
	connect(this, &DeviceControl::sigHighresImu, m_pDebugDialog, &DeviceDebug::onUpdateHighresImu);
	connect(this, &DeviceControl::sigAttitude, m_pDebugDialog, &DeviceDebug::onUpdateAttitude);
	connect(this, &DeviceControl::sigLogMessage, m_pDebugDialog, &DeviceDebug::onMessageData);
	connect(this, &DeviceControl::sigLocalPosition, m_pDebugDialog, &DeviceDebug::onUpdateLocalPosition);

	m_heartbeatStatus.bNoHeartbeatIng = false;
	connect(&m_timerHeartbeat, &QTimer::timeout, [this]() {
		//未启用心跳判断
		if (false == m_bHeartbeatEnable) return;
		//无人机校准中无心跳数据
		if (m_heartbeatStatus.bNoHeartbeatIng) return;
		unsigned int nLastTime = m_timerHeartbeat.property("time").toUInt();
		if ((QDateTime::currentDateTime().toTime_t() - nLastTime) > 10) {
			if (isConnectDevice()) {
				m_timerHeartbeat.setProperty("time", QDateTime::currentDateTime().toTime_t());
				emit sigLogMessage("无人机无响应主动断开连接");
				connectDevice();
			}
		}
		});
	connect(ui.btnRemove, &QToolButton::clicked, [this]() {
		emit sigRemoveDevice(ui.labelDeviceName->text()); 
		});
	connect(ui.btnLED, &QAbstractButton::clicked, [this]() {
		//手动点亮无人机灯光
		int res = Fun_MAV_LED_MODE();
		if (_DeviceStatus::DeviceDataSucceed != res) {
			_ShowErrorMessage(tr("灯光控制出错:") + Utility::waypointMessgeFromStatus(_DeviceLed, res));
		}
		});
	connect(ui.btnStop, &QToolButton::clicked, [this]() {
		int res = Fun_MAV_QUICK_STOP();
		if (res == _DeviceStatus::DeviceDataSucceed) return;
		_ShowErrorMessage(getName() + tr("急停") + Utility::waypointMessgeFromStatus(_DeviceQuickStop, res));
		});
	setName(name);
	setIp(ip);
	setStartLocation(x, y);
	ui.checkBox->setChecked(true);
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

void DeviceControl::setCurrentTime(unsigned int time)
{
	m_nCurrentMusicTime = time;
}

DeviceDebug* DeviceControl::getDeviceDebug()
{
	return m_pDebugDialog;
}

void DeviceControl::setStartLocation(long x, long y)
{
	onUpdateLocation(0, x, y, 0);
	setX(x);
	setY(y);
}

_stDeviceCurrentStatus DeviceControl::getCurrentStatus()
{
	return m_deviceStatus;
}

void DeviceControl::enableControl(bool enable)
{
	ui.btnRemove->setEnabled(enable);
}

bool DeviceControl::isCheckDevice()
{
	return ui.checkBox->isChecked();
}

void DeviceControl::setChcekStatus(bool check)
{
	if(check) ui.checkBox->setCheckState(Qt::Checked);
	else ui.checkBox->setCheckState(Qt::Unchecked);
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
	m_heartbeatStatus.bNoHeartbeatIng = false;
	//绑定回调函数
	m_pHvTcpClient->onConnection = std::bind(&DeviceControl::hvcbConnectionStatus, this, std::placeholders::_1);
	m_pHvTcpClient->onMessage = std::bind(&DeviceControl::hvcbReceiveMessage, this, std::placeholders::_1, std::placeholders::_2);
	m_pHvTcpClient->onWriteComplete = std::bind(&DeviceControl::hvcbWriteComplete, this, std::placeholders::_1, std::placeholders::_2);
	m_pHvTcpClient->setConnectTimeout(2 * 1000);

	m_bUploadFinished = false;
	m_bTimeSync = false;
	m_bPrepareTakeoff = false;

	//配置自动重连模式
	hv::ReconnectInfo reconn;
	reconn.min_delay = 2 * 1000;
	reconn.max_delay = 10 * 1000;
	reconn.delay_policy = 1;
	m_pHvTcpClient->setReconnect(&reconn);
	m_pHvTcpClient->start();
	return true;
}

//断开连接
void DeviceControl::disconnectDevice()
{
	m_bUploadFinished = false;
	m_bTimeSync = false;
	m_bPrepareTakeoff = false;
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
bool DeviceControl::sendMessage(bool again, QByteArray data)
{
	if (data.isEmpty()) return false;
	if (!isConnectDevice()) return false;
	int len = m_pHvTcpClient->send(data.data(), data.length());
	QString text = again ? "重发消息" : "发送消息";
	qDebug() << getName() << text << len << data.toHex().toUpper();
	if (len != data.length()) return false;
	return true;
}

//无人机设置是否启用心跳
void DeviceControl::setHeartbeatEnable(bool enable)
{
	m_bHeartbeatEnable = enable;
}

bool DeviceControl::isUploadWaypoint()
{
	return m_bUploadFinished;
}

bool DeviceControl::isTimeSync()
{
	return m_bTimeSync;
}

void DeviceControl::clearTimeSyncStatus()
{
	m_bTimeSync = false;
}

unsigned int DeviceControl::getTimeSyncUTC()
{
	return m_nTimeSynsUTC;
}

bool DeviceControl::isPrepareTakeoff()
{
	return m_bPrepareTakeoff;
}

//无人机设置模式
int DeviceControl::Fun_MAV_CMD_DO_SET_MODE(float Mode, bool wait, bool again)
{
	//参数2,3,4,5,6,7无效
	//1姿态模式|2定高模式|3航点模式
	QByteArray arrData = mavCommandLongToBuffer(Mode, 0, 0, 0, 0, 0, 0, MAV_CMD_DO_SET_MODE);
	int res = MavSendCommandLongMessage(tr("准备起飞"), MAV_CMD_DO_SET_MODE, arrData, again ? arrData : "", false);
	if (DeviceDataSucceed == res) m_bPrepareTakeoff = true;
	return res;
}

int DeviceControl::DeviceMavWaypointStart(QVector<NavWayPointData> data)
{
	if (!isConnectDevice()) return DeviceUnConnect;
	int count = data.count();
	if (count <= 0) return DeviceDataError;
	//标记航点下发是否进行中
	if (m_bWaypointSending) return DeviceMessageSending;
	m_bWaypointSending = true;
	QString text = tr("准备上传舞步数量")+QString::number(count);
	emit sigLogMessage(text);
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
	ui.stackedWidgetStatus->setCurrentIndex(1);
	m_currentWaypointData = data;
	ui.btnRemove->setEnabled(false);
	ui.checkBox->setEnabled(false);
	ResendMessage* pMessageThread = new ResendMessage(text, ui.labelDeviceName->text(), _MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrData, MAVLINK_MSG_ID_MISSION_COUNT);
	connect(this, SIGNAL(sigCommandResult(QString, int, int)), pMessageThread, SLOT(onResult(QString, int, int)));
	connect(pMessageThread, SIGNAL(sigSendMessage(bool, QByteArray)), this, SLOT(sendMessage(bool, QByteArray)));
	connect(pMessageThread, SIGNAL(finished()), this, SLOT(onWaypointStart()));
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
	int res = MavSendCommandLongMessage(tr("起飞"), MAV_CMD_NAV_TAKEOFF_LOCAL, arrData, again ? arrData : "", wait);
	if (DeviceDataSucceed == res) {
		m_bPrepareTakeoff = false;
		m_bUploadFinished = false;
		m_bTimeSync = false;
	}
	return res;
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
	int res = MavSendCommandLongMessage(tr("校准"), MAV_CMD_PREFLIGHT_CALIBRATION, arrData, again ? arrData : "", wait);
	if (DeviceDataSucceed == res) {
		m_heartbeatStatus.nLastTime = QDateTime::currentDateTime().toTime_t();
		m_heartbeatStatus.bNoHeartbeatIng = true;
	}
	return res;
}

//无人机列队
int DeviceControl::Fun_MAV_Defined_Queue(int x, int y)
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, x, y, MAV_CMD_USER_2);
	return MavSendCommandLongMessage(tr("列队"), MAV_CMD_USER_2, arrData, arrData, false);
}

//无人机回收
int DeviceControl::Fun_MAV_Defined_Regain(int x, int y)
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, x, y, MAV_CMD_USER_1);
	return MavSendCommandLongMessage(tr("回收"), MAV_CMD_USER_1, arrData, arrData, false);
}

//无人机灯光
int DeviceControl::Fun_MAV_LED_MODE()
{
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_WAYPOINT_USER_4);
	return MavSendCommandLongMessage(tr("灯光"), MAV_CMD_WAYPOINT_USER_4, arrData, arrData, false);
}

int DeviceControl::Fun_MAV_TimeSync()
{
	//不可以重发，发送后立刻返回
	m_bTimeSync = false;
	m_nTimeSynsUTC = 0;
	QByteArray arrData = mavCommandLongToBuffer(0, 0, 0, 0, 0, 0, 0, MAV_CMD_WAYPOINT_USER_5);
	int res = MavSendCommandLongMessage(tr("定桩授时"), MAV_CMD_WAYPOINT_USER_5, arrData, arrData, false);
	//记录定桩授时状态
	if (DeviceDataSucceed == res) {
		m_bTimeSync = true;
		m_nTimeSynsUTC = QDateTime::currentDateTime().toTime_t();
	}
	return res;
}

void DeviceControl::onUpdateBatteryStatus(float voltages, float battery, unsigned short electric)
{
	QString text = "<font color=%1>";
	if (electric > 80) {
		text = text.arg("#467FC1");
	}
	else if (electric > 60) {
		text = text.arg("#DAA520");
	}
	else {
		text = text.arg("#FF0000");
	}
	ui.labelBattery->setText(QString(text+tr("%1%</font>")).arg(electric));
	m_pDebugDialog->onSetBatteryStatus(voltages, battery, electric);
}

void DeviceControl::onUpdateLocation(unsigned int time, int x, int y, int z)
{
	QString text = QString("X:%1 Y:%2 Z:%3").arg(x).arg(y).arg(z);
	ui.labelLocation->setText(text);
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
			unsigned int time = m_nCurrentMusicTime;
			mavlink_message_t message;
			mavlink_heartbeat_t heard;
			heard.custom_mode = time;
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
	QString qstrStart(_DeviceLogPrefix_);
	QString qstrEnd(_DeviceLogEnd_);
	QString qstrData = QString::fromLocal8Bit(arrData);
	//qDebug() << getName()<< "收到消息" << arrData.toHex().toUpper();
	if (qstrData.contains(qstrStart)) {
		//校准时没有心跳数据
		emit sigUpdateHeartbeat();
		QString qstrLog = qstrData;
		while (qstrLog.contains(qstrStart)){
			int nStart = qstrLog.indexOf(qstrStart);
			int nEnd = qstrLog.indexOf(qstrEnd) + qstrEnd.length();
			if (qstrEnd.length() == nEnd) nEnd = qstrLog.length() - nStart;
			QString temp = qstrLog.mid(nStart, nEnd);
			qstrLog.remove(temp);
			emit sigLogMessage(temp);
		}
	}
	//解包
	//重置通道状态
	mavlink_reset_channel_status(m_nMavChan);
	mavlink_message_t msg;
	mavlink_status_t status;
	for (int i = 0; i < arrData.length(); i++) {
		uint8_t par = mavlink_parse_char(m_nMavChan, arrData[i], &msg, &status);
		if (0 == par) continue;
		//qDebug() << getName() << "解包后消息ID" << msg.msgid;
		if (m_heartbeatStatus.bNoHeartbeatIng) {
			//长时间无心跳时则恢复心跳超时判断
			if ((QDateTime::currentDateTime().toTime_t() - m_heartbeatStatus.nLastTime) > 60) {
				m_heartbeatStatus.bNoHeartbeatIng = false;
			}
		}
		switch (msg.msgid)
		{
		case MAVLINK_MSG_ID_HEARTBEAT://心跳
			mavlink_heartbeat_t heart;
			mavlink_msg_heartbeat_decode(&msg, &heart);
			emit sigUpdateHeartbeat();
			//qDebug() << getName() << "心跳消息" << msg.msgid;
			break;
		case MAVLINK_MSG_ID_MISSION_COUNT:  //航点起始应答
		{
			emit sigUpdateHeartbeat();
			qDebug() << getName() << "起始航点应答" << msg.msgid;
			mavlink_mission_count_t missionCount;
			mavlink_msg_mission_count_decode(&msg, &missionCount);
			uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = { 0 };
			int len = mavlink_msg_to_send_buffer(buffer, &msg);
			QByteArray temp = QByteArray((char*)buffer, len);
			//0成功,成功后发送航点数据
			emit sigCommandResult(ui.labelDeviceName->text(), missionCount.count > 0 ? 0 : -1, MAVLINK_MSG_ID_MISSION_COUNT);
			emit sigMessageByte(temp, true, MAVLINK_MSG_ID_MISSION_COUNT);
			break;
		}
		case MAVLINK_MSG_ID_MISSION_ACK:    //航点应答
		{
			emit sigUpdateHeartbeat();
			qDebug() << getName() << "航点应答" << msg.msgid;
			mavlink_mission_ack_t ack;
			mavlink_msg_mission_ack_decode(&msg, &ack);
			uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = { 0 };
			int len = mavlink_msg_to_send_buffer(buffer, &msg);
			QByteArray temp = QByteArray((char*)buffer, len);
			emit sigCommandResult(ui.labelDeviceName->text(), ack.type, MAVLINK_MSG_ID_MISSION_ACK);
			emit sigMessageByte(temp, true, MAVLINK_MSG_ID_MISSION_ACK);
			break;
		}
		case MAVLINK_MSG_ID_COMMAND_ACK:	//命令应答
		{	//起飞降落等实时控制指令应答
			emit sigUpdateHeartbeat();
			mavlink_command_ack_t ack;
			mavlink_msg_command_ack_decode(&msg, &ack);
			uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = { 0 };
			int len = mavlink_msg_to_send_buffer(buffer, &msg);
			QByteArray temp = QByteArray((char*)buffer, len);
			emit sigMessageByte(temp, true, MAVLINK_MSG_ID_COMMAND_ACK);
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
			if (n > 100) n = 100;
			emit sigBatteryStatus((float)v / 1000, (float)b / 1000, n);
			m_deviceStatus.battery = n;
			//qDebug() << getName() << "电池信息" << msg.msgid;
			break;
		}
		case MAVLINK_MSG_ID_ATTITUDE:	//姿态角
			mavlink_attitude_t attitude;
			mavlink_msg_attitude_decode(&msg, &attitude);
			m_deviceStatus.roll = attitude.roll;
			m_deviceStatus.pitch = attitude.pitch;
			m_deviceStatus.yaw = attitude.yaw;
			emit sigAttitude(attitude.time_boot_ms, attitude.roll, attitude.pitch, attitude.yaw);
			//qDebug() << getName() << "姿态角" << msg.msgid;
			break;
		case MAVLINK_MSG_ID_LOCAL_POSITION_NED: //位置信息
		{
			mavlink_local_position_ned_t t;
			mavlink_msg_local_position_ned_decode(&msg, &t);
			//单位转换成cm
			m_deviceStatus.x = QString::number(t.x * 100, 'f', 0).toInt();
			m_deviceStatus.y = QString::number(t.y * 100, 'f', 0).toInt();
			m_deviceStatus.z = QString::number(t.z * 100, 'f', 0).toInt();
			emit sigLocalPosition(t.time_boot_ms, m_deviceStatus.x, m_deviceStatus.y, m_deviceStatus.z);
			//qDebug() << getName() << "位置信息" << msg.msgid;
			break;
		}
		case MAVLINK_MSG_ID_HIGHRES_IMU:	//IMU数据
		{
			mavlink_highres_imu_t t;
			mavlink_msg_highres_imu_decode(&msg, &t);
			QList<float> list;
			list << t.xacc << t.yacc << t.zacc << t.xgyro << t.ygyro << t.zgyro << t.xmag << t.ymag << t.zmag;
			emit sigHighresImu(t.time_usec, list);
			//qDebug() << getName() << "IMU数据" << msg.msgid;
			break;
		}
		case MAVLINK_MSG_ID_SYS_STATUS:
		{
			//包含固件版本信息
			mavlink_sys_status_t t;
			mavlink_msg_sys_status_decode(&msg, &t);
			unsigned int nVersionFirmware = t.onboard_control_sensors_present;
		}
		default:
			//qWarning() << getName() << "未知消息" << msg.msgid;
			break;
		}
	}
	//qDebug() << getName() << "数据包处理完成" << msg.msgid << status.packet_rx_success_count;
}

//已经发送的数据
void DeviceControl::hvcbWriteComplete(const hv::SocketChannelPtr& channel, hv::Buffer* buf)
{//子线程
	if (buf->size() <= 0) return;
	QByteArray arrBuf((char*)buf->data(), buf->size());
	if (arrBuf.isEmpty()) return;
	//qDebug() << getName() << "已发送消息" << arrBuf.toHex().toUpper();
}

QByteArray DeviceControl::mavMessageToBuffer(mavlink_message_t mesage)
{
	//需要加锁
	QMutexLocker locker(&m_mutexMavMessageToBurrer);
	uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = { 0 };
	int len = mavlink_msg_to_send_buffer(buffer, &mesage);
	QByteArray arrData = QByteArray((char*)buffer, len);
	emit sigMessageByte(arrData, false, mesage.msgid);
	//qDebug() << "准备数据" << getName() << arrData.toHex().toUpper();
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
	connect(pMessageThread, SIGNAL(sigSendMessage(bool, QByteArray)), this, SLOT(sendMessage(bool, QByteArray)));
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

QByteArray DeviceControl::getWaypointData(float param1, float param2, float param3, float param4, int32_t x, int32_t y, float z, uint16_t seq, unsigned int commandID, unsigned int again)
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
	mission.command = commandID;
	mission.target_system = _DeviceSYS_ID_;
	mission.target_component = _DeviceCOMP_ID_;
	mission.current = again;
	//if(0 == again) qDebug() << "上传航点到无人机" << getName() << seq << param1 << param2 << param3 << param4 << x << y << z << commandID;
	//else qDebug() << "重新上传航点到无人机" << getName() << seq << param1 << param2 << param3 << param4 << x << y << z << commandID;
	int len = mavlink_msg_mission_item_int_encode(_DeviceSYS_ID_, _DeviceCOMP_ID_, &msg, &mission);
	return mavMessageToBuffer(msg);
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
	ResendMessage* pMessageThread = new ResendMessage(tr("准备结束上传航点"), ui.labelDeviceName->text(), _MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrAgainData, MAVLINK_MSG_ID_MISSION_ACK);
	//接收设备回复的消息响应
	connect(this, SIGNAL(sigCommandResult(QString, int, int)), pMessageThread, SLOT(onResult(QString, int, int)));
	connect(pMessageThread, SIGNAL(sigSendMessage(bool, QByteArray)), this, SLOT(sendMessage(bool, QByteArray)));
	pMessageThread->start();
	//阻塞等待结果
	while (!pMessageThread->isFinished()) {
		QApplication::processEvents();
	}
	//线程结果后查询返回结果
	int res = pMessageThread->getResult();
	delete pMessageThread;
	pMessageThread = nullptr;
	//整个上传航点过程结束
	m_bWaypointSending = false;
	m_bUploadFinished = true;
	ui.btnRemove->setEnabled(true);
	ui.checkBox->setEnabled(true);
	m_nCurrentWaypontIndex = -1;
	ui.stackedWidgetStatus->setCurrentIndex(0);	
	QString text = "舞步上传" + Utility::getControlError(res);
	emit sigLogMessage(text);
	emit sigWaypointFinished(getName(), DeviceDataSucceed == res, text);
}

void DeviceControl::onUpdateHeartBeat()
{
	unsigned int nTime = QDateTime::currentDateTime().toTime_t();
	m_timerHeartbeat.setProperty("time", nTime);
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
		qInfo() << name << "无人机连接成功";
		m_timerHeartbeat.setProperty("time", QDateTime::currentDateTime().toTime_t());
		m_timerHeartbeat.start(_DeviceHeartbeatInterval_);
		QPixmap pixmap(":/res/images/uavgreen.png");
		ui.labelStatus->setPixmap(pixmap.scaled(ui.labelStatus->size()));
	}
	else {
		qInfo() << name << "无人机连接断开";
		m_timerHeartbeat.stop();
		QPixmap pixmap(":/res/images/uavred.png");
		ui.labelStatus->setPixmap(pixmap.scaled(ui.labelStatus->size()));
		//onUpdateLocation(0, 0, 0, 0);
		onUpdateBatteryStatus(0, 0, 0);
	}
}

void DeviceControl::onWaypointStart()
{
	ResendMessage* pMessage = dynamic_cast<ResendMessage*>(sender());
	if (!pMessage) return;
	int res = pMessage->getResult();
	QString text;
	if (DeviceDataSucceed != res) {
		m_bWaypointSending = false;
		ui.btnRemove->setEnabled(true);
		ui.checkBox->setEnabled(true);
		m_nCurrentWaypontIndex = -1;
		ui.stackedWidgetStatus->setCurrentIndex(0);
		text = tr("准备上传航点失败") + Utility::getControlError(res);
		emit sigLogMessage(text);
		emit sigWaypointFinished(getName(), false, text);
		return;
	}
	delete pMessage;

	text = tr("准备上传0号航点");
	emit sigLogMessage(text);
	//先发送0号全零航点，验证是否可以上传航点
	QByteArray arrData = getWaypointData(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	QByteArray arrAgainData = getWaypointData(0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
	ResendMessage* pWaypointThread = new ResendMessage(text, ui.labelDeviceName->text(), _MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrAgainData, MAVLINK_MSG_ID_MISSION_ACK);
	connect(this, SIGNAL(sigCommandResult(QString, int, int)), pWaypointThread, SLOT(onResult(QString, int, int)));
	connect(pWaypointThread, SIGNAL(sigSendMessage(bool, QByteArray)), this, SLOT(sendMessage(bool, QByteArray)));
	connect(pWaypointThread, SIGNAL(finished()), this, SLOT(onWaypointNext()));
	pWaypointThread->start();
}

void DeviceControl::onWaypointNext()
{
	ResendMessage* pMessage = dynamic_cast<ResendMessage*>(sender());
	if (!pMessage) return;
	int res = pMessage->getResult();
	delete pMessage;
	if (DeviceDataSucceed != res) {
		m_bWaypointSending = false;
		ui.btnRemove->setEnabled(true);
		ui.checkBox->setEnabled(true);
		ui.stackedWidgetStatus->setCurrentIndex(0);
		QString error = QString("%1号航点上传失败%2").arg(m_nCurrentWaypontIndex).arg(Utility::getLedError(res));
		if (m_nCurrentWaypontIndex < 0) error = tr("0号航点上传失败") + Utility::getLedError(res);
		emit sigLogMessage(error);
		emit sigWaypointFinished(getName(), false, error);
		return;
	}

	if (m_nCurrentWaypontIndex > m_currentWaypointData.count()) {
		DeviceMavWaypointEnd(m_currentWaypointData.count());
		return;
	}
	//上传进度
	QString text = QString("%1号航点上传完成，准备上传%2号航点").arg(m_nCurrentWaypontIndex + 1).arg(m_nCurrentWaypontIndex + 2);
	if (m_nCurrentWaypontIndex < 0) text = "0号航点上传完成";
	else if ((m_nCurrentWaypontIndex + 1) == m_currentWaypointData.count()) text = QString::number(m_nCurrentWaypontIndex + 1)+"号航点上传完成，准备结束上传舞步";
	emit sigLogMessage(text);
	m_nCurrentWaypontIndex++;
	ui.progressBar->setValue(m_nCurrentWaypontIndex);
	QByteArray arrData;
	QByteArray arrAgainData;
	if(m_nCurrentWaypontIndex < m_currentWaypointData.count()){
		NavWayPointData temp = m_currentWaypointData.at(m_nCurrentWaypontIndex);
		//与设备通讯协议中规定X与Y值需要*1000
		arrData = getWaypointData(temp.param1, temp.param2, temp.param3, temp.param4, temp.x * 1000, temp.y * 1000, temp.z, m_nCurrentWaypontIndex + 1, temp.commandID, 0);
		arrAgainData = getWaypointData(temp.param1, temp.param2, temp.param3, temp.param4, temp.x * 1000, temp.y * 1000, temp.z, m_nCurrentWaypontIndex + 1, temp.commandID, 1);
	}
	else {
		//航点下发完成后需要发送结束信息
		DeviceMavWaypointEnd(m_currentWaypointData.count());
		return;
	}
	ResendMessage* pWaypointThread = new ResendMessage(tr("上传舞步"), ui.labelDeviceName->text(), _MavWaypointRetryNum_, _MavWaypointTimeout_, arrData, arrAgainData, MAVLINK_MSG_ID_MISSION_ACK);
	connect(this, SIGNAL(sigCommandResult(QString, int, int)), pWaypointThread, SLOT(onResult(QString, int, int)));
	connect(pWaypointThread, SIGNAL(sigSendMessage(bool, QByteArray)), this, SLOT(sendMessage(bool, QByteArray)));
	connect(pWaypointThread, SIGNAL(finished()), this, SLOT(onWaypointNext()));
	pWaypointThread->start();
}

void DeviceControl::onMessageThreadFinished()
{
	ResendMessage* pMessage = dynamic_cast<ResendMessage*>(sender());
	if (!pMessage) return;
	emit sigConrolFinished(pMessage->getCommandName(), pMessage->getResult(), "");
	delete pMessage;
}
