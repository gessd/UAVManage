#pragma once

#include "mavlink_types.h"
#include "common/mavlink.h"

//设备系统ID
#define _DeviceSYS_ID_  1
//设备组件ID
#define _DeviceCOMP_ID_ 1
//消息重发标识
#define _MavResendFlag_ 1

//设备TCP连接端口
#define _DevicePort_ 8899

//急停指令消息ID
#define _MavStopFlyMessageID_ 15
//急停超时间隔
#define _MavQuickStopFlyTimeout_ 1000
//指令消息重发发送次数
#define _MavLinkResendNum_ 2
//航点重复次数
#define _MavWaypointRetryNum_ 2
//心跳消息时间间隔 毫秒
#define _DeviceHeartbeatInterval_ 1000
//指令消息重发间隔 毫秒
#define _NkCommandResendInterval_ 3000
//上传航点超时 毫秒
#define _MavWaypointTimeout_ 2000

//日志消息前缀
#define _DeviceLogPrefix_   "日志"
//日志消息结束
#define _DeviceLogEnd_   0x0D0A