#pragma once

#define _TcpPort_ 23366
#define _VerNum_  1.1
#define _TabName_ "qz"
#define _Ver_    "version"
#define _Tag_    "tag"
#define _Time_   "time"
#define _ID_     "id"
#define _Data_   "data"

enum _MsgID {
	_3dDeviceList = 101,
	_3dDeviceAdd = 102,
	_3dDeviceRemove = 103,
	_3dDeviceRename = 104,
	_3dDeviceLocation = 105,
	_3dDeviceWaypoint = 106,
	//_3dDeviceBattery = 107,
	_3dDeviceMusicPath = 108,
	_3dDeviceAction = 201,
	_3dDeviceTime = 202
};

//三维消息头部尾部字符
#define _MsgHead '*'
#define _MsgTail '#'