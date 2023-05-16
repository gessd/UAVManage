#pragma once
#include <QDir>
//程序版本号
#define _MajorNumber_ 1
#define _MinorNumber_ 0
#define _BuildNumber_ 9
//新程序下载存放文件夹
#define _NewVersionPath_	"/update"
#define _VersionFile_		"version.ini"
//服务器地址
#define _ServerUrl_ "http://123.127.240.26:8086/downloads/software/UAVManage/"

//blockly交互端口
#define _WebSocketPort_ 25252

//调试版本
//#define _DebugApp_

//项目后缀名
#define _ProjectSuffix       ".qz"
#define _PyFileSuffix_       ".py"
#define _PyManualSuffix_     ".mp"
#define _BlcoklyFileSuffix_  ".blockly"
#define _ProjectDirName_ "/code/"

//设备名称前缀
#define _DeviceNamePrefix_ "无人机"
//无人机默认起飞高度 厘米
#define _TakeoffLocalHeight_ 100

//航点增加时间
#define _WaypointUseTime_

//XML文件
// <?xml version="1.0" encoding="UTF-8"?>
// <root>
//     场地
//     <place x="10" y="10"/> 
//     无人名称，IP,初始位置
//     <device name="无人机" ip="" x="0" y="0"/>
// </root>
//tinyxml2文件名不能使用UTF-8编码,否则无法打开
#define _XMLNameCoding_    "GBK"
#define _XMLVersion_       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
#define _ElementRoot_      "root"
#define _ElementPlace_     "place"
#define _ElementDevice_    "device"
#define _ElementMusic_      "music"
#define _AttributeX_       "x"
#define _AttributeY_       "y"
#define _AttributeZ_       "z"
#define _AttributeName_    "name"
#define _AttributeIP_      "ip"


enum _WaypointType {
	_WaypointFly		= 16,		//飞行动作航点
	_WaypointSpeed		= 31000,	//设置飞行速度
	_WaypointRevolve	= 31001,	//旋转信息
	_WaypointHover		= 31002,	//悬停信息
	_WaypointLedStatus	= 31003,    //LED灯状态信息
	_WaypointStart		= 31004,	//初始位置信息
	_WaypointTime		= 31005,	//时间信息
	_WaypointLedColor	= 31006,	//LED灯颜色
	_WaypointFlyLand	= 23		//降落

};

enum _CalibrationEnum {
	_Gyro = 0,			//陀螺校准
	_Magnetometer,		//磁罗盘校准 
	_MagEnable,			//磁罗盘使能开关
	_Remote,			//无效值
	_Accelerometer,		//加计校准
	_Compmot,			//无效值
	_Baro				//电调校准
};

enum _DeviceStatus
{
	//程序内部错误
	DeviceMessageToimeout = -404,
	DeviceLowBattery = -5,
	DeviceMessageSending = -4,
	DeviceDataError = -3,
	DeviceUnConnect = -2,
	DeviceWaiting = -1,
	DeviceDataSucceed = 0,
	//无人机返回错误
	FlyNoWaypoint = 1,	//航线错误
	Flying = 2,			//正在飞行中
	FlyNoPrepare = 3	//没有准备起飞
};

enum PythonRunState
{
	PythonRunNone = 0,
	PythonSuccessful,
	PythonError,
	PythonFileError,
	PythonTimeout,
	PythonCodeError,
	PythonWaypointError,
	PythonWaypointNull
};

enum _AllDeviceCommand {
	_DeviceTakeoffLocal = 1, //起飞
	_DeviceLandLocal,		 //降落
	_DeviceQuickStop,		 //急停
	_DevicePrepare,			 //准备起飞
	_DeviceQueue,			 //列队
	_DeviceRegain,			 //回收
	_DeviceLed,				//LED控制
	_DeviceWaypoint,		//舞步
	_DeviceCalibration		//设备校准
};

static QString AppVersion() {
	return QString("%1.%2.%3").arg(_MajorNumber_).arg(_MinorNumber_).arg(_BuildNumber_);
}

static bool deleteDir(const QString& path)
{
	if (path.isEmpty()) {
		return false;
	}
	QDir dir(path);
	if (!dir.exists()) {
		return true;
	}
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
	QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
	foreach(QFileInfo file, fileList)
	{ //遍历文件信息
		if (file.isFile()) { // 是文件，删除
			file.dir().remove(file.fileName());
		}
		else { // 递归调用函数，删除子文件夹
			deleteDir(file.absoluteFilePath());
		}
	}
	return dir.rmpath(dir.absolutePath()); // 这时候文件夹已经空了，再删除文件夹本身
}
class Utility :public QObject
{
public:
	static QString waypointMessgeFromStatus(int control, int status)
	{
		if (status <= 0) return getControlError(status);
		switch (control) {
		case _DeviceTakeoffLocal: return getTakeoffError(status); break;
		case _DeviceLandLocal: return getLandError(status); break;
		case _DeviceQuickStop: return getQueueError(status); break;
		case _DevicePrepare: return getReadyError(status); break;
		case _DeviceQueue: return getQueueError(status); break;
		case _DeviceRegain: return getRegainError(status); break;
		case _DeviceLed: return getLedError(status); break;
		case _DeviceWaypoint: return getWaypointError(status); break;
		}
		return tr("控制出错");
	}
	static QString getControlError(int type)
	{
		QString qstrMessage = tr("控制出错");
		switch (type)
		{
		case DeviceMessageToimeout:
			qstrMessage = QObject::tr("没有回应");
			break;
		case DeviceLowBattery:
			qstrMessage = QObject::tr("电量过低");
			break;
		case DeviceMessageSending:
			qstrMessage = QObject::tr("进行中");
			break;
		case DeviceDataError:
			qstrMessage = QObject::tr("数据错误");
			break;
		case DeviceUnConnect:
			qstrMessage = QObject::tr("设备未连接");
			break;
		case DeviceWaiting:
			qstrMessage = QObject::tr("操作超时");
			break;
		case DeviceDataSucceed:
			qstrMessage = QObject::tr("成功");
			break;
		}
		return qstrMessage;
	}
	static QString getWaypointDescribeFromID(int type)
	{
		QString qstrText = "未定义类型";
		switch (type) {
		case _WaypointFly: qstrText = QObject::tr("飞行"); break;
		case _WaypointSpeed: qstrText = QObject::tr("设置速度"); break;
		case _WaypointRevolve: qstrText = QObject::tr("旋转"); break;
		case _WaypointHover: qstrText = QObject::tr("悬停"); break;
		case _WaypointLedStatus: qstrText = QObject::tr("LED灯模式"); break;
		case _WaypointStart: qstrText = QObject::tr("初始位置"); break;
		case _WaypointTime: qstrText = QObject::tr("时间信息"); break;
		case _WaypointFlyLand: qstrText = QObject::tr("降落"); break;
		case _WaypointLedColor: qstrText = QObject::tr("LED灯颜色"); break;
		}
		return qstrText;
	}
	//准备起飞
	static QString getReadyError(int type) {
		QString qstrText = "操作出错";
		return qstrText;
	}
	//起飞
	static QString getTakeoffError(int type) {
		QString qstrMessage = "操作出错";
		switch (type) {
		case FlyNoWaypoint:
			qstrMessage = QObject::tr("没有航线或航线错误");
			break;
		case Flying:
			qstrMessage = QObject::tr("正在飞行中");
			break;
		case FlyNoPrepare:
			qstrMessage = QObject::tr("没有准备起飞");
			break;
		}
		return qstrMessage;
	}
	//降落
	static QString getLandError(int type) {
		QString qstrText = "操作出错";
		switch (type)
		{
		case 1: qstrText = tr("飞机并未起飞"); break;
		case 2: qstrText = tr("飞机当前正在降落"); break;
		}
		return qstrText;
	}
	//停止
	static QString getStopError(int type) {
		QString qstrText = "操作出错";
		return qstrText;
	}
	//航点
	static QString getWaypointError(int type) {
		QString qstrText = "操作出错";
		return qstrText;
	}
	//列队
	static QString getQueueError(int type) {
		QString qstrText = "操作出错";
		return qstrText;
	}
	//回收
	static QString getRegainError(int type) {
		QString qstrText = "操作出错";
		return qstrText;
	}
	//LED控制
	static QString getLedError(int type) {
		QString qstrText = "操作出错";
		return qstrText;
	}
};

//航点属性
typedef struct __NavWayPointData
{
	float param1;       //停留时间 秒
	float param2;       //接受半径
	float param3;       //飞行时间 秒
	float param4;       //偏转角度
	int x;				//厘米
	int y;				//厘米
	float z;			//厘米	
	unsigned int commandID;
	QString message;
	__NavWayPointData() {
		param2 = 0.20;   //接受半径需要有默认值 TODO 参数单位
		param1 = param3 = param4 = z = 0.0;
		x = y = 0;
		//[16=航点信息||31000=速度信息||31001= 转向信息||31002=延时停留时间||31003=LED状态灯||31004=初始位置||31005=时间信息]
		//根据ID不同参数对应不同意义
		commandID = 16; 
	}
}NavWayPointData;

#define _name2str(name) (#name)
//结构体属性转换成字符串, 必须以.分割,否则因列表越界造成崩溃
#define _attribute2str(name) (QString(#name).split(".").at(1))
//2022-10-20后改为使用json格式传送数据
// c++ 为 websocketserver
// blocily 为 websocketclient
//Blockly通讯消息格式
/**
 * c++ to blockly
 * 1.更加编程区域
 * {"msgID":1,"xml":"data", "name":"无人机名称"}
 * 2.清空编程区域
 * {"msgID":2}
 * 3.设置空间范围
 * {"msgID":3, "x":1000, "y":10000, "z":10000}
 */
/**
 * blockly to c++
 * 1.更加编程区域
 * {"msgID":1,"xml":"data", "python":"data"}
 */
#define _WMID "msgID"
enum _TypeWMID {
	_WIDUpdate = 1,	//blockly区域更新
	_WIDClear = 2,	//情况编程区域
	_WIDSet = 3,	//设置场地大小
	_WIDManual = 4	//手动python代码更新
};