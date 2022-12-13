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
	_3dDeviceInit = 101,
	_3dDeviceLocation = 102
};

//三维消息头部尾部字符
#define _MsgHead '*'
#define _MsgTail '#'