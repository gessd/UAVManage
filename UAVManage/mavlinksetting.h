#pragma once

#include "mavlink_types.h"
#include "common/mavlink.h"

//设备系统ID
#define _DeviceSYS_ID_  1
//设备组件ID
#define _DeviceCOMP_ID_ 1
//消息重发标识
#define _NavResendFlag_ 1

//急停指令消息ID
#define _NavStopFlyMessageID_ 15
//设备TCP连接端口
#define _DevicePort_ 8899
//心跳消息时间间隔
#define _DeviceHeartbeatInterval_ 500

//指令消息重发发送次数
#define _NavLinkResendNum_ 3
//指令消息重发间隔 毫秒
#define _NkCommandResendInterval_ 3000

//下发航点超时 毫秒
#define _NavWaypointTimeout_ 2000
//急停超时间隔
#define _NavQuickStopFlyTimeout_ 1000
//航点重复次数
#define _NavWaypointRetryNum_ 3

