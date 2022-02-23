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
	_3dDeviceAdd,
	_3dDeviceRemove,
	_3dDeviceRename,
	_3dDeviceLocation,
	_3dDeviceWaypoint,
	_3dDeviceBattery,
	_3dDeviceMusicPath,
	_3dDeviceAction = 201,
	_3dDeviceTime
};
