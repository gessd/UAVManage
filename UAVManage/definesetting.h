#pragma once

#define _WebSocketPort_ 25252

//项目后缀名
#define _ProjectSuffix  ".qz"
//默认项目名称
#define _ProjectName    "temp"

//设备名称前缀
#define _DeviceNamePrefix_ "无人机"

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
#define _AttributeX_       "x"
#define _AttributeY_       "y"
#define _AttributeZ_       "z"
#define _AttributeName_    "name"
#define _AttributeIP_      "ip"

