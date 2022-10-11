#pragma once

#define _WebSocketPort_ 25252

//项目后缀名
#define _ProjectSuffix       ".qz"
#define _PyFileSuffix_       ".py"
#define _BlcoklyFileSuffix_  ".blockly"
//默认项目名称
#define _ProjectName    "temp"
#define _ProjectDirName_ "/code/"

//设备名称前缀
#define _DeviceNamePrefix_ "无人机"
//无人机默认起飞高度 厘米
#define _TakeoffLocalHeight_ 100

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

//航点属性
typedef struct __NavWayPointData
{
	float param1;       //停留时间
	float param2;       //接受半径
	float param3;       //轨迹控制
	float param4;       //偏转角度
	int x;
	int y;
	float z;
	unsigned int commandID;
	__NavWayPointData() {
		param2 = 0.20;   //偏转角度需要有默认值
		param1 = param3 = param4 = z = 0.0;
		x = y = 0;
		//[16=航点信息||31000=速度信息||31001= 转向信息||31002=延时停留时间||31003=LED状态灯||31004=初始位置||31005=时间信息]
		//根据ID不同参数对应不同意义
		commandID = 16; 
	}
}NavWayPointData;


